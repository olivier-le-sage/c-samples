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
    fifo = initFIFO("fifo.txt", PRODUCER, 5);
    if (fifo != NULL) {
        signal(SIGINT, intHandler);
        int data;
        while(true) {
            printf("Please enter an integer:\n");
            if (1 == scanf("%d", &data)) {
                if (-1 == push(fifo, data)) printf("Couldn't push. FIFO must be full.\n");
            }
        }
        closeFIFO(fifo);
    }
}
