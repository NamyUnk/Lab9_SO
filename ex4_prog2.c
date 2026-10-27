#define _POSIX_C_SOURCE 200809L
#include "shared_data.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

/* Exercicio 4, prog2: le a mensagem compartilhada e escreve uma resposta. */
static void pause_short(void) { struct timespec t = {0, 10000000L}; nanosleep(&t, NULL); }

int main(void) {
    int fd = open(SHARED_FILE, O_RDWR);
    if (fd == -1) { perror("open (execute prog1 primeiro)"); return 1; }
    shared_data *d = mmap(NULL, sizeof(*d), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (d == MAP_FAILED) { perror("mmap"); return 1; }

    while (!d->message_ready) pause_short();
    __sync_synchronize();
    printf("prog2 recebeu: %s\\n", d->message);
    snprintf(d->response, MESSAGE_SIZE, "Resposta do prog2!");
    __sync_synchronize();
    d->response_ready = 1;
    munmap(d, sizeof(*d));
    return 0;
}
