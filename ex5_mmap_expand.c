#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/* Exercicio 5: aumenta arquivo, desfaz o mapeamento e mapeia o tamanho novo. */
int main(void) {
    const char *file = "ex5_arquivo.txt";
    const char *initial = "Texto inicial";
    const char *extra = " - texto acrescentado";
    size_t oldlen = strlen(initial), addlen = strlen(extra), newlen = oldlen + addlen;

    int fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) { perror("open"); return 1; }
    if (write(fd, initial, oldlen) != (ssize_t)oldlen) { perror("write"); close(fd); return 1; }

    char *p = mmap(NULL, oldlen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    printf("Antes: %.*s\\n", (int)oldlen, p);
    if (munmap(p, oldlen) == -1) { perror("munmap"); close(fd); return 1; }

    /* Sem aumentar o arquivo, acessar bytes depois do EOF pode causar SIGBUS. */
    if (ftruncate(fd, (off_t)newlen) == -1) { perror("ftruncate"); close(fd); return 1; }
    p = mmap(NULL, newlen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap novo"); close(fd); return 1; }
    memcpy(p + oldlen, extra, addlen);
    if (msync(p, newlen, MS_SYNC) == -1) { perror("msync"); munmap(p, newlen); close(fd); return 1; }
    printf("Depois: %.*s\\n", (int)newlen, p);
    munmap(p, newlen); close(fd);
    return 0;
}
