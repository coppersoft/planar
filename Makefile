CC=vc +kick13
CFLAGS=-I$(NDK_INC) -c99 -O2
LDFLAGS=-lamiga -lauto

all: bitplanes sprites vblank
	$(CC) $(CFLAGS) $(LDFLAGS) bitplanes.o sprites.o vblank.o main.c -o main.exe

# -c è "do not link", salva come .o
bitplanes:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/bitplanes.c -c -o bitplanes.o

sprites:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/sprites.c -c -o sprites.o

vblank:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/vblank.c -c -o vblank.o

clean:
	rm *.o
	rm main.exe
