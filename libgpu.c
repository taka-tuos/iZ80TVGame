#include "libgpu.h"

char *font6x8;
char *font8x8;

void libgpu_init(void)
{
	FILE *fp0 = fopen("932-6x8a.bin","rb");
	FILE *fp1 = fopen("932-8x8j.bin","rb");

	font6x8 = (char *)malloc(2048);
	font8x8 = (char *)malloc(65536);

	fread(font6x8,sizeof(char),2048,fp0);
	fread(font8x8,sizeof(char),65536,fp1);
	fclose(fp0);
	fclose(fp1);
}


void libgpu_putc(SDL_Surface *sdl_screen, int x, int y, int c, char *font)
{
	int i;
	int *p, d;
	
	for (i = 0; i < 8; i++) {
		if(y + i < sdl_screen->h && y + i >= 0) {
			p = ((int *)sdl_screen->pixels) + (y + i) * (sdl_screen->pitch >> 2) + x;
			d = font[i];
			if(x + 0 < (sdl_screen->pitch >> 2) && x + 0 >= 0) if((d & 0x80) != 0) { p[0] = c; }
			if(x + 1 < (sdl_screen->pitch >> 2) && x + 1 >= 0) if((d & 0x40) != 0) { p[1] = c; }
			if(x + 2 < (sdl_screen->pitch >> 2) && x + 2 >= 0) if((d & 0x20) != 0) { p[2] = c; }
			if(x + 3 < (sdl_screen->pitch >> 2) && x + 3 >= 0) if((d & 0x10) != 0) { p[3] = c; }
			if(x + 4 < (sdl_screen->pitch >> 2) && x + 4 >= 0) if((d & 0x08) != 0) { p[4] = c; }
			if(x + 5 < (sdl_screen->pitch >> 2) && x + 5 >= 0) if((d & 0x04) != 0) { p[5] = c; }
			if(x + 6 < (sdl_screen->pitch >> 2) && x + 6 >= 0) if((d & 0x02) != 0) { p[6] = c; }
			if(x + 7 < (sdl_screen->pitch >> 2) && x + 7 >= 0) if((d & 0x01) != 0) { p[7] = c; }
		}
	}
	
	return;
}

void libgpu_puts(SDL_Surface *sdl_screen, int x, int y, int c, char *sz)
{
	char *font;
	unsigned char *s = (unsigned char *)sz;
	int k,t;
	int langbyte1 = 0;
	
	for (; *s != 0x00; s++) {
		if (langbyte1 == 0) {
			if((0x81 <= *s && *s <= 0x9f) || (0xe0 <= *s && *s <= 0xfc)) {
				langbyte1 = *s;
			} else {
				libgpu_putc(sdl_screen, x, y, c, font6x8 + *s * 8);
				x += 6;
			}
		} else {
			if(0x81 <= langbyte1 && langbyte1 <= 0x9f) {
				k = (langbyte1 - 0x81) * 2;
			} else {
				k = (langbyte1 - 0xe0) * 2 + 62;
			}
			if(0x40 <= *s && *s <= 0x7e) {
				t = *s - 0x40;
			} else if (0x80 <= *s && *s <= 0x9e) {
				t = *s - 0x80 + 63;
			} else {
				t = *s - 0x9f;
				k++;
			}
			langbyte1 = 0;
			libgpu_putc(sdl_screen, x, y, c, font8x8 + (k * 94 + t) * 8);
			x += 8;
		}
	}
	return;
}

void libgpu_line2d(SDL_Surface *sdl_screen, int x0, int y0, int x1, int y1, int col)
{
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;
 
	for(;;){
		if(x0 >= 0 && x0 < (sdl_screen->pitch >> 2) && y0 >= 0 && y0 < sdl_screen->h)((int *)sdl_screen->pixels)[y0 * (sdl_screen->pitch >> 2) + x0] = col;
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void libgpu_box2d(SDL_Surface *sdl_screen, int x0, int y0, int x1, int y1, int col)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++) {
			if(x >= 0 && x < (sdl_screen->pitch >> 2) && y >= 0 && y < sdl_screen->h)((int *)sdl_screen->pixels)[y * (sdl_screen->pitch >> 2) + x] = col;
		}
	}
	return;
}
