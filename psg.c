#include "iZ80TVGame.h"
#include "SDL.h"

SDL_AudioSpec Desired;
SDL_AudioSpec Obtained;

char psg_buffer[22050];

extern int cpu_sndptr;

#define FREQ	22050

unsigned long psg_step = 0;

void callback_psg(void *unused, Uint8 *stream, int len)
{
	int i;
	int chs;
	Sint8 *frames = (Sint8 *) stream;
	int framesize = len;
	for (i = 0; i < framesize; i++, psg_step++) {
		frames[i] = psg_buffer[((psg_step/4)%FREQ)] ? 127 : -128;
	}
}

void reset_psg(void)
{
	SDL_PauseAudio(1);
	psg_step = 0;
	memset(psg_buffer,0,FREQ);
}

void init_psg(void)
{
	reset_psg();

	Desired.freq= FREQ; /* Sampling rate: 22050Hz */
	Desired.format= AUDIO_S8; /* 16-bit signed audio */
	Desired.channels= 1; /* Mono */
	Desired.samples= FREQ/30; /* Buffer size: 2205000B = 10 sec. */
	Desired.callback = callback_psg;
	Desired.userdata = NULL;

	SDL_OpenAudio(&Desired, &Obtained);
}
