#ifndef SHARED_DATA_H
#define SHARED_DATA_H
#define SHARED_FILE "troca_mmap.dat"
#define MESSAGE_SIZE 256
typedef struct {
    volatile int message_ready;
    volatile int response_ready;
    char message[MESSAGE_SIZE];
    char response[MESSAGE_SIZE];
} shared_data;
#endif
