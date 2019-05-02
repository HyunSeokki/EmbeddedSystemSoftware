CC = arm-none-linux-gnueabi-gcc
#CC = gcc

main : main.c
	$(CC) --static -o main main.c
	$(CC) --static -o inproc inproc.c
	$(CC) --static -o outproc outproc.c

clean :
	rm main inproc outproc
