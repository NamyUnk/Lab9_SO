#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* Exercicio 2: imprime o arquivo acessando-o por um mapeamento de memoria. */
int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Uso: %s <arquivo>\\n", argv[0]); return 1; }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) { perror("open"); return 1; }

    struct stat sb;
    if (fstat(fd, &sb) == -1) { perror("fstat"); close(fd); return 1; }
    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, "Nao e um arquivo regular.\\n"); close(fd); return 1;
    }
    if (sb.st_size == 0) { close(fd); return 0; }

    char *p = mmap(NULL, (size_t)sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    close(fd);

    if (fwrite(p, 1, (size_t)sb.st_size, stdout) != (size_t)sb.st_size) {
        perror("fwrite"); munmap(p, (size_t)sb.st_size); return 1;
    }
    if (munmap(p, (size_t)sb.st_size) == -1) { perror("munmap"); return 1; }
    return 0;
}
