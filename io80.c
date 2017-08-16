#include "iZ80TVGame.h"

extern byte *z80_memory;
extern int psg_buffer[22050];
extern SDL_Joystick *joy;
extern int cpu_wait;

int cpu_beep = 0;

byte io80_readm(int param, ushort address)
{
	return z80_memory[address];
}

void io80_writem(int param, ushort address, byte data)
{
	if(address >= 0x8000) z80_memory[address] = data;
}

byte io80_readp(int param, ushort address)
{
	int ret = 0;

	switch(address & 0xff) {
	case 0x00:
		if(SDL_JoystickGetButton(joy,1)) {
			ret |= 0x20;	// TRIG1
		}
		if(SDL_JoystickGetButton(joy,2)) {
			ret |= 0x10;	// TRIG2
		}
		if(SDL_JoystickGetButton(joy,9)) {
			ret |= 0x08;	// right
		}
		if(SDL_JoystickGetButton(joy,7)) {
			ret |= 0x04;	// left
		}
		if(SDL_JoystickGetButton(joy,6)) {
			ret |= 0x02;	// down
		}
		if(SDL_JoystickGetButton(joy,8)) {
			ret |= 0x01;	// up
		}
		ret ^= 0xff;
		break;
	case 0x02:
		ret = cpu_beep;
		break;
	default:
		ret = 0xff;
		break;
	}

	return ret;
}

void io80_writep(int param, ushort address, byte data)
{
	switch(address & 0xff) {
	case 0x02:
		cpu_beep = data & 1;
		break;
	}
}
