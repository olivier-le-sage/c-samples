/* A FIFO data structure designed for a threading context with one producer and
 * one consumer, using no semaphores or mutexes.
 *
 * - pop() returns the oldest element in the fifo.
 * - push(int) adds an element to the end of the fifo.
 *
 * The FIFO is backed up by a file on the system so that the producer and
 *  consumer threads can work separately (similar to a unix pipe).
 *
 * One flaw of this approach: the producer and consumer threads must have
 *  already agreed on both the filename and capacity of the fifo. Otherwise this
 *  won't work.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <sys/stat.h>

typedef enum {PRODUCER, CONSUMER} access_mode;

typedef struct FIFO {
    int front;
    int rear;
    int size;               // current number of elements
    unsigned int capacity;  // max elements
    char* file;             // file path/name
    FILE* fp;
    access_mode mode;       // read/write <=> consumer/producer (resp.)
} FIFO;

/* Note: other utility functions could be added. */
FIFO* initFIFO(char* filename, access_mode mode, unsigned int capacity);
void closeFIFO(FIFO* fifo);
int push(FIFO* fifo, uint32_t elem);
int pop(FIFO* fifo);
bool is_empty(FIFO* fifo);
bool is_full(FIFO* fifo);
