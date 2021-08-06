CC=vc +kick13
CFLAGS=-I$(NDK_INC) -c99 #-O2 -I../include
LDFLAGS=-lamiga -lauto

all:
	$(CC) $(CFLAGS) $(LDFLAGS) main.c -o main.exe
