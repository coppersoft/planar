#include <clib/exec_protos.h>

static volatile ULONG *custom_vposr = (volatile ULONG *) 0xdff004;

void wait_vblank();