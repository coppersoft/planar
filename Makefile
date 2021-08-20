CC=vc +kick13
CFLAGS=-I$(NDK_INC) -c99 -O2 #-I../include
LDFLAGS=-lamiga -lauto

all: putils
	$(CC) $(CFLAGS) $(LDFLAGS) putils.o main.c -o main.exe

# -c è "do not link", salva come .o
putils:
	$(CC) $(CFLAGS) $(LDFLAGS) putils.c -c -o putils.o

clean:
	rm *.o
	rm main.exe
