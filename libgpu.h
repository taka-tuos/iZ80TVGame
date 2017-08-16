#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>

void libgpu_init(void);
void libgpu_putc(SDL_Surface *sdl_screen, int x, int y, int c, char *font);
void libgpu_puts(SDL_Surface *sdl_screen, int x, int y, int c, char *sz);
void libgpu_line2d(SDL_Surface *sdl_screen, int x0, int y0, int x1, int y1, int col);
void libgpu_box2d(SDL_Surface *sdl_screen, int x0, int y0, int x1, int y1, int col);
