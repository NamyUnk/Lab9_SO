#define _POSIX_C_SOURCE 200809L
#include "shared_data.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>

/* Exercicio 4, prog1: escreve uma mensagem e espera a resposta de prog2. */
static void pause_short(void) { struct timespec t = {0, 10000000L}; nanosleep(&t, NULL); }

int main(void) {
    int fd = open(SHARED_FILE, O_RDWR | O_CREAT, 0600);
    if (fd == -1) { perror("open"); return 1; }
    if (ftruncate(fd, (off_t)sizeof(shared_data)) == -1) {
        perror("ftruncate"); close(fd); return 1;
    }
    shared_data *d = mmap(NULL, sizeof(*d), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (d == MAP_FAILED) { perror("mmap"); return 1; }

    d->message_ready = 0; d->response_ready = 0;
    snprintf(d->message, MESSAGE_SIZE, "Ola do prog1!");
    d->response[0] = '\\0';
    __sync_synchronize();
    d->message_ready = 1;
    printf("prog1 enviou: %s\\n", d->message);
    while (!d->response_ready) pause_short();
    __sync_synchronize();
    printf("prog1 recebeu: %s\\n", d->response);
    munmap(d, sizeof(*d));
    return 0;
}
