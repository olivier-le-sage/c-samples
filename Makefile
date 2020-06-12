CC=gcc
CFLAGS = -Wall -I. 

consumer.exe: consumer.c fifo.c
	$(CC) -o consumer consumer.c fifo.c
	
producer.exe: producer.c fifo.c
	$(CC) -o producer producer.c fifo.c
	
capitalizer.exe: capitalizer.c
	$(CC) -o capitalizer capitalizer.c
	
all: consumer.exe producer.exe capitalizer.exe
.PHONY: all