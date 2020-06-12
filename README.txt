Some code samples in C.

Currently includes two examples:

* A FIFO data structure which uses a mechanism similar to a Unix pipe to allow IPC between a producer and consumer thread.
* A parser that takes an input text file and tries to capitalize the first letter of the first word of each sentence.

Notes:

- Originally compiled with MinGW on Windows
- Run consumer.exe and producer.exe in separate terminals
- When Ctrl-C is hit to exit the program, the local files used by fifo.c are automatically removed.
- Sample file input.txt is provided with capitalizer.c as an example
