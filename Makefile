CC=vc +kick13
CFLAGS=-I$(NDK_INC) -c99 -O2
LDFLAGS=-lamiga -lauto
ASM=vasmm68k_mot -Fhunk

all: init bitplanes sprites blitter vblank disk input debug screen audio interrupt collision animation ptplay
	$(CC) $(CFLAGS) $(LDFLAGS) init.o bitplanes.o sprites.o vblank.o blitter.o disk.o input.o debug.o screen.o audio.o interrupt.o collision.o animation.o ptplay.o main.c -o main.exe

# -c è "do not link", salva come .o
init:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/init.c -c -o init.o

bitplanes:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/bitplanes.c -c -o bitplanes.o

sprites:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/sprites.c -c -o sprites.o

vblank:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/vblank.c -c -o vblank.o

blitter:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/blitter.c -c -o blitter.o

disk:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/disk.c -c -o disk.o

input:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/input.c -c -o input.o

debug:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/debug.c -c -o debug.o

screen:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/screen.c -c -o screen.o

audio:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/audio.c -c -o audio.o

interrupt:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/interrupt.c -c -o interrupt.o

collision:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/collision.c -c -o collision.o

ptplay:
	$(ASM) ./ptplayer/ptplayer.asm -o ptplay.o

animation:
	$(CC) $(CFLAGS) $(LDFLAGS) ./planar/animation.c -c -o animation.o

ptplayer:
	$(ASM) ./music/ptplayer.asm -o ptplayer.o

clean:
	rm *.o
	rm main.exe
