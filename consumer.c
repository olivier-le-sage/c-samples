/* Test code for fifo.c
 */

#include <signal.h>
#include "fifo.h"

FIFO* fifo;

void intHandler(int sig) {
    signal(sig, SIG_IGN);
    if (NULL != fifo) closeFIFO(fifo);
    exit(0);
}

int main() {
    signal(SIGINT, intHandler);
    fifo = initFIFO("fifo.txt", CONSUMER, 5);
    if (fifo != NULL) {
        int data;
        while(true) {
            data = pop(fifo);
            if (data != INT_MIN) printf("Retrieved %d from the fifo.\n", data);
        }
        closeFIFO(fifo);
    }
}
