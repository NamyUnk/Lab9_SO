#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

/* Exercicio 3: calcula paginas e compara escrita por mmap e write. */
static double now_seconds(void) {
    struct timeval tv; gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}
static int write_all(int fd, const char *buf, size_t n) {
    size_t done = 0;
    while (done < n) {
        ssize_t w = write(fd, buf + done, n - done);
        if (w < 0) { if (errno == EINTR) continue; return -1; }
        done += (size_t)w;
    }
    return 0;
}
int main(int argc, char **argv) {
    size_t len = 10u * 1024u * 1024u;
    if (argc > 2) { fprintf(stderr, "Uso: %s [tamanho_em_bytes]\\n", argv[0]); return 1; }
    if (argc == 2) {
        char *end = NULL; unsigned long long v = strtoull(argv[1], &end, 10);
        if (!end || *end || v == 0 || v > SIZE_MAX) { fprintf(stderr, "Tamanho invalido.\\n"); return 1; }
        len = (size_t)v;
    }
    long ps = sysconf(_SC_PAGESIZE);
    if (ps <= 0) { perror("sysconf"); return 1; }
    size_t pages = len / (size_t)ps + (len % (size_t)ps != 0);
    printf("Tamanho: %zu bytes\\nTamanho da pagina: %ld bytes\\nPaginas necessarias: %zu\\n", len, ps, pages);

    char *src = malloc(len);
    if (!src) { perror("malloc"); return 1; }
    memset(src, 'X', len);

    int fd = open("ex3_mmap.bin", O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd == -1 || ftruncate(fd, (off_t)len) == -1) {
        perror("open/ftruncate mmap"); if (fd != -1) close(fd); free(src); return 1;
    }
    char *p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); close(fd); free(src); return 1; }
    double start = now_seconds();
    memcpy(p, src, len);
    if (msync(p, len, MS_SYNC) == -1 || fsync(fd) == -1) {
        perror("msync/fsync"); munmap(p, len); close(fd); free(src); return 1;
    }
    double tmmap = now_seconds() - start;
    munmap(p, len); close(fd);

    fd = open("ex3_write.bin", O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) { perror("open write"); free(src); return 1; }
    start = now_seconds();
    if (write_all(fd, src, len) == -1 || fsync(fd) == -1) {
        perror("write/fsync"); close(fd); free(src); return 1;
    }
    double twrite = now_seconds() - start;
    close(fd); free(src);

    printf("mmap + msync + fsync: %.6f s\\n", tmmap);
    printf("write + fsync:        %.6f s\\n", twrite);
    puts(tmmap < twrite ? "Neste teste, mmap foi mais rapido." :
         twrite < tmmap ? "Neste teste, write foi mais rapido." : "Tempos iguais.");
    puts("Arquivos gerados: ex3_mmap.bin e ex3_write.bin");
    return 0;
}
