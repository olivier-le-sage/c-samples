/* A FIFO data structure designed for a threading context with one producer and
 * one consumer, using no semaphores or mutexes.
 *
 * - pop() returns the oldest element in the fifo
 * - push(int) adds an element to the end of the fifo
 *
 * This FIFO is backed up by a file on the system so that the producer and
 *  consumer threads can work separately (similar to a unix pipe).
 *
 * One flaw of this approach: the producer and consumer threads must have
 *  already agreed on both the filename and capacity of the fifo. Otherwise this
 *  won't work.
 *
 * NB: Without using a proper kernel/OS library for IPC, the reliability of this
 *  data structure isn't guaranteed.
 */

#include "fifo.h"

#define BUFFER_SIZE 40
#define NUM_HEX_IN_UINT32 32/4

/* checks if a file already exists */
static bool file_exists(char *filename) {
    struct stat buffer;
    return (stat (filename, &buffer) == 0);
}

/* Initializes a fifo using the specified file. Returns NULL in case of failure.*/
FIFO* initFIFO(char* filename, access_mode mode, unsigned int capacity) {


    FIFO* fifo = (FIFO*) malloc(sizeof(FIFO));

    if (mode == PRODUCER) {
        if (file_exists(filename)) return NULL; // Don't overwrite existant files.
        fifo->fp = fopen(filename, "w");
        if (fifo->fp == NULL) return NULL; // could not open file
    } else if (mode == CONSUMER) {
        fifo->fp = fopen(filename, "r");
        if (fifo->fp == NULL) return NULL; // could not open file
    } else {
        return NULL; // invalid input
    }

    fifo->front = 0;
    fifo->rear  = 0;
    fifo->size  = 0;
    fifo->capacity = capacity;
    fifo->file = filename;
    fifo->mode = mode;
    return fifo;
}

/* cleanup method. Has to be run, or else subsequent runs may have issues. */
void closeFIFO(FIFO* fifo) {
    fclose(fifo->fp);
    if (fifo->mode == CONSUMER) remove("front.txt");
    if (fifo->mode == PRODUCER) remove("rear.txt");
    remove(fifo->file);
    free(fifo);
}

bool is_empty(FIFO* fifo) {
    return (fifo->size == 0);
}

bool is_full(FIFO* fifo) {
    return (fifo->size >= fifo->capacity);
}

/* Returns -1 on failure and 0 on success */
/* fifo data is stored as uint32_t to avoid integer size problems */
int push(FIFO* fifo, uint32_t elem) {
    if (fifo->mode == CONSUMER) return -1; // consumer can't push

    /* check if the consumer changed the front */
    /* if it hasn't (or the file doesn't exist), assume size is still valid */
    FILE* temp_fp = fopen("front.txt", "r");
    char buffer[BUFFER_SIZE];
    if (temp_fp != NULL) {
        if (NULL != fgets(buffer, BUFFER_SIZE, temp_fp)) {
            int front = atoi(buffer); // atoi() can be replaced if desired
            if (front != fifo->front) {
                /* adjust front accordingly */
                fifo->front = front;
                /* re-adjust size based on rear and front */
                if (fifo->rear < fifo->front) {
                    fifo->size = fifo->capacity + (fifo->rear - fifo->front);
                } else {
                    fifo->size = fifo->rear - fifo->front;
                }
            }
        }
        fclose(temp_fp);
    }

    if (is_full(fifo)) return -1; // do nothing if the fifo is full

    /* write the new element to the rear of the fifo */
    rewind(fifo->fp); // rewind for good measure
    fseek(fifo->fp, fifo->rear*(NUM_HEX_IN_UINT32+2), SEEK_SET);
    fprintf(fifo->fp, "%08X\n", elem); // EOL is used as the delimiter
    fflush(fifo->fp); // flush output buffer to file

    /* note: because the rear pointer loops, past/invalid values in the fifo
               will be overwritten progressively as required. This looping
               keeps the fifo from growing indefinitely. */
    fifo->rear = (fifo->rear + 1) % fifo->capacity;
    fifo->size++;

    /* use a temp file to inform the consumer that the size has changed. */
    temp_fp = fopen("rear.txt", "w");
    fprintf(temp_fp, "%d", fifo->rear);
    fflush(temp_fp); // flush output buffer to file
    fclose(temp_fp);

    return 0;
}

/* Returns INT_MIN on error */
int pop(FIFO* fifo) {
    if (fifo->mode == PRODUCER) return INT_MIN; // producer can't pop

    /* check if the producer changed the rear */
    /* if it hasn't (or the file doesn't exist), assume size is still valid */
    FILE* temp_fp = fopen("rear.txt", "r");
    char buffer[BUFFER_SIZE];
    if (temp_fp != NULL) {
        if (NULL != fgets(buffer, BUFFER_SIZE, temp_fp)) {
            int rear = atoi(buffer); // atoi() can be replaced if desired

            if (rear != fifo->rear) {
                /* adjust rear accordingly */
                fifo->rear = rear;
                /* re-adjust size based on rear and front */
                if (fifo->rear < fifo->front) {
                    fifo->size = fifo->capacity + (fifo->rear - fifo->front);
                } else {
                    fifo->size = fifo->rear - fifo->front;
                }
            }
        }
        fclose(temp_fp);
    }

    if (is_empty(fifo)) return INT_MIN; // do nothing if the fifo is empty

    /* read from file */
    int i = 0;
    uint32_t elem;
    rewind(fifo->fp);
    while(NULL != fgets(buffer, BUFFER_SIZE, fifo->fp)) { // read one line at a time
        if (i == fifo->front) { // find the first element of the fifo
            sscanf(buffer, "%08X\n", &elem); // read it
            break; // stop looping once the element is found
        } else {
            i++; // loop until we get to the right line
        }
    }

    /* note: the front pointer moves forward without deleting past values.
                This allows file access safety -- the consumer only
                reads from the file, and only the producer can write. */
    fifo->front = (fifo->front + 1) % fifo->capacity;
    fifo->size--;

    /* use a temp file to inform the producer that the size has changed. */
    temp_fp = fopen("front.txt", "w");
    fprintf(temp_fp, "%d", fifo->front);
    fflush(temp_fp);
    fclose(temp_fp);

    return elem;
}
