CC=vc +kick13
CFLAGS=-I$(NDK_INC) -c99 -O2
LDFLAGS=-lamiga -lauto

all: bitplanes sprites blitter vblank disk input debug
	$(CC) $(CFLAGS) $(LDFLAGS) bitplanes.o sprites.o vblank.o blitter.o disk.o input.o debug.o main.c -o main.exe

# -c è "do not link", salva come .o
bitplanes:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/bitplanes.c -c -o bitplanes.o

sprites:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/sprites.c -c -o sprites.o

vblank:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/vblank.c -c -o vblank.o

blitter:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/blitter.c -c -o blitter.o

disk:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/disk.c -c -o disk.o

input:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/input.c -c -o input.o

debug:
	$(CC) $(CFLAGS) $(LDFLAGS) ./utils/debug.c -c -o debug.o

clean:
	rm *.o
	rm main.exe
