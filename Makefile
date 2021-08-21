CC=vc +kick13
CFLAGS=-I$(NDK_INC) -c99 -O2
LDFLAGS=-lamiga -lauto

all: bitplanes
	$(CC) $(CFLAGS) $(LDFLAGS) bitplanes.o main.c -o main.exe

# -c è "do not link", salva come .o
bitplanes:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/bitplanes.c -c -o bitplanes.o

clean:
	rm *.o
	rm main.exe
