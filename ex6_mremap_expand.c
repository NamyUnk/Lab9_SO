#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/* Exercicio 6: mesma ideia do ex. 5, expandindo o mapeamento com mremap. */
int main(void) {
    const char *file = "ex6_arquivo.txt";
    const char *initial = "Texto inicial";
    const char *extra = " - texto acrescentado";
    size_t oldlen = strlen(initial), addlen = strlen(extra), newlen = oldlen + addlen;

    int fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) { perror("open"); return 1; }
    if (write(fd, initial, oldlen) != (ssize_t)oldlen) { perror("write"); close(fd); return 1; }

    char *p = mmap(NULL, oldlen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    printf("Antes: %.*s\\n", (int)oldlen, p);

    /* mremap aumenta o mapeamento, mas nao aumenta o arquivo: use ftruncate. */
    if (ftruncate(fd, (off_t)newlen) == -1) { perror("ftruncate"); munmap(p, oldlen); close(fd); return 1; }
    char *q = mremap(p, oldlen, newlen, MREMAP_MAYMOVE);
    if (q == MAP_FAILED) { perror("mremap"); munmap(p, oldlen); close(fd); return 1; }

    memcpy(q + oldlen, extra, addlen);
    if (msync(q, newlen, MS_SYNC) == -1) { perror("msync"); munmap(q, newlen); close(fd); return 1; }
    printf("Depois: %.*s\\n", (int)newlen, q);
    munmap(q, newlen); close(fd);
    return 0;
}
