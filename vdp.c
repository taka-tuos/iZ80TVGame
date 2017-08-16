#include "iZ80TVGame.h"

extern byte *z80_memory;

int rgb_table[] = { 0x00, 0xff };

void vdp_grefresh(SDL_Surface *sdl_screen)
{
	unsigned int *fb = ((unsigned long *)sdl_screen->pixels);
	int offset;
	for(int y = 0, offset = 0xc03c; y < 210; y++, offset+=30) {
		for(int x = 2; x < 25; x++) {
			unsigned char data = z80_memory[offset+x];
			for(int i = 0; i < 8; i++) {
				int c = ((data >> (i & 7)) & 1) * 0xffffff;
				int ptr = (y+31) * (sdl_screen->pitch/4) + ((x*8+i)+144-32);
				
				fb[ptr+0] = c;
				//fb[ptr+1] = c;
			}
		}
	}
}
