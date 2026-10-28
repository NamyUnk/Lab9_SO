#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

static void *memorypool;

static void *myallocate(int n) {
    if (n <= 0) {
        errno = EINVAL;
        return (void *)-1;
    }

    return sbrk((intptr_t)n);
}

static void initmemorypool(void) {
    memorypool = sbrk(0);
}

static int resetmemorypool(void) {
    return brk(memorypool);
}

int main(void) {
    initmemorypool();

    int *a = myallocate(10 * (int)sizeof(int));

    if (a == (void *)-1) {
        perror("sbrk");
        return 1;
    }

    for (int i = 0; i < 10; ++i) {
        a[i] = (i + 1) * 10;
    }

    printf("Valores: ");
    for (int i = 0; i < 10; ++i) {
        printf("%d%s", a[i], i == 9 ? "\n" : " ");
    }

    char *msg = myallocate(32);

    if (msg == (void *)-1) {
        perror("sbrk");
        resetmemorypool();
        return 1;
    }

    snprintf(msg, 32, "Memoria alocada com sbrk()");
    printf("Mensagem: %s\n", msg);

    fflush(stdout);

    if (resetmemorypool() == -1) {
        perror("brk");
        return 1;
    }

    _exit(0);
}