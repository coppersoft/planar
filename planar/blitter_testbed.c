// https://replit.com/languages/c

int main() {

	int x = 33;
	int shift = x & 0xf;
	int words_offset = x >> 4;
	
    printf("Shift: %d \n",shift);
	printf("Words_offset: %d\n",words_offset);

    unsigned short   bltcon0_value = 0x0fe2;     // 0 shift momentaneamente, f tutti i canali
                                        // e2 i minterm per cookie cut
    unsigned short   bltcon1_value = 0x0;

	shift = shift << 12;
	
    bltcon0_value |= shift;
	bltcon1_value |= shift;

	printf ("bltcon0: 0x%04x\n",bltcon0_value);
	printf ("bltcon1: 0x%04x\n",bltcon1_value);
	
    return 0;
}
  