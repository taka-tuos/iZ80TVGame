#include "iZ80TVGame.h"
#include "libgpu.h"

#define FPS				30.0f
#define MHZ				4.0f

byte *z80_memory;

extern char psg_buffer[22050];
extern int cpu_beep;

int cpu_sndptr = 0;
int cpu_wait = 0;

int poll_event(SDL_Event *sdl_event)
{
	if(SDL_PollEvent(sdl_event)) {
		switch (sdl_event->type) {
		case SDL_QUIT:
			return 1;
		}
	}

	return 0;
}

void adjustFPS(void) {
	static unsigned long maetime=0;
	static int frame=0;
	long sleeptime;
	if(!maetime) SDL_GetTicks();
	frame++;
	sleeptime=(frame<FPS)?
		(maetime+(long)((float)frame*(1000.0f/FPS))-SDL_GetTicks()):
		(maetime+1000-SDL_GetTicks());
	if(sleeptime>0)SDL_Delay(sleeptime);
	if(frame>=FPS) {
		frame=0;
		maetime=SDL_GetTicks();
	}
}

unsigned int sec_tstates = 0;
int psg_sync = 0;
int z80_tstates = 0;

void z80_execute(Z80_STATE *z80)
{
	int elim;
	unsigned int tstates;
	
	if(!psg_sync) SDL_PauseAudio(0);
	psg_sync = 1;
	
	tstates = z80_tstates;
	
	cpu_wait = 0;
	
	for(elim = 0; (int)((unsigned long)z80_tstates - tstates) < MHZ*1000.0f*1000.0f/FPS;elim++) {
		int ptbk = (int)((unsigned long)z80_tstates - sec_tstates) / 181;
		int bpbk = cpu_beep;
		
		ptbk = ptbk < 22050 ? ptbk : 22049;
		
		z80_tstates += Z80Emulate(z80,1,NULL);
		
		z80_tstates += cpu_wait;
		cpu_wait = 0;
		
		int lm = (int)((unsigned long)z80_tstates - sec_tstates);
		cpu_sndptr = lm / 181;
		cpu_sndptr = cpu_sndptr < 22050 ? cpu_sndptr : 22049;
		if(cpu_sndptr-ptbk > 0) memset(psg_buffer+ptbk,bpbk,cpu_sndptr-ptbk);
		psg_buffer[cpu_sndptr] = cpu_beep;
	}
}

int atoi16(char *sz)
{
	if(sz[strlen(sz)-1] == 0x0a) sz[strlen(sz)-1] = 0;
	if(sz[strlen(sz)-2] == 0x0d) sz[strlen(sz)-2] = 0;
	
	int n = 0;
	int l = (strlen(sz)-1)*4;
	char *p = sz;
	
	for(;*p!=0;p++) {
		if(*p >= '0' && *p <= '9') n |= (*p - '0'     ) << l;
		if(*p >= 'A' && *p <= 'F') n |= (*p - 'A' + 10) << l;
		if(*p >= 'a' && *p <= 'f') n |= (*p - 'a' + 10) << l;
		
		l -= 4;
	}
	
	return n;
}

int hexread(unsigned char *dat, char *fname)
{
	FILE *r=fopen(fname,"rt");/* インプットファイルオープン (第2引数) */
	
	int cf = 1;
	
	int lb = 0;
	
	int lc = 0;
	
	while(!feof(r) && cf) {
		char ls[256];
		
		fgets(ls,256,r);
		lc++;
		
		char *p = ls;
		
		if(*p == ':') {
			p++;
			
			char h_bc[32];
			char h_ad[32];
			char h_rt[32];
			char h_dt[256];
			
			strncpy(h_bc,p  ,2);
			strncpy(h_ad,p+2,4);
			strncpy(h_rt,p+6,2);
			
			h_bc[2] = 0;
			h_ad[4] = 0;
			h_rt[2] = 0;
			
			int n_bc = atoi16(h_bc);
			int n_rt = atoi16(h_rt);
			
			int i;
			
			switch(n_rt) {
			case 0x00:
				lb = n_bc + atoi16(h_ad)+1;
				strncpy(h_dt,p+8,n_bc*2);
				for(i = 0; i < n_bc; i++) {
					char hexb[32];
					
					strncpy(hexb,h_dt+i*2,2);
					
					dat[atoi16(h_ad)+i] = atoi16(hexb);
				}
				break;
			case 0x01:
				cf = 0;
				break;
			default:
				printf("warning : unknown record type at line %d : %x(\"%s\"\n",lc, n_rt, h_rt);
				break;
			}
		}
	}
	
	fclose(r);
	
	return lb;
}

struct dirent *roms[512];

SDL_Joystick *joy;

void chld_rom(SDL_Surface *sdl_screen)
{
	SDL_Event sdl_event;

	DIR *dir = opendir("./roms/");
	
	int i;
	for(i = 0; i < 512; i++) {
		roms[i] = 0;
	}

	readdir(dir);
	readdir(dir);

	for(i = 0; i < 512; i++) {
		struct dirent *p = readdir(dir);
		if(!p) break;
		roms[i] = (struct dirent *)malloc(sizeof(struct dirent));
		memcpy(roms[i],p,sizeof(struct dirent));
	}
	
	closedir(dir);
	
reread:
	;
	
	int j = 0;

	int k1=0,k2=0;
	int l = 0;

	while(1) {
		memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);

		if(poll_event(&sdl_event)) {
			SDL_Quit();
			exit(0);
		}

		libgpu_puts(sdl_screen,0,0,0xffffff,"Please select the ROM.");

		for(i = l; i < 512; i++) {
			if(!roms[i]) break;

			libgpu_puts(sdl_screen,12,(i-l)*8+16,0xffffff,roms[i]->d_name);
		}

		libgpu_puts(sdl_screen,0,j*8+16,0xffffff,">");

		if(SDL_JoystickGetButton(joy,8)) {
			if(k1 == 0) j--;
			k1 = 1;
		} else {
			k1 = 0;
		}
		if(SDL_JoystickGetButton(joy,6)) {
			if(k2 == 0) j++;
			k2 = 1;
		} else {
			k2 = 0;
		}
		if(SDL_JoystickGetButton(joy,1)) break;

		if(j < 0 && l > 0) l--;
		if(j > 21 && l < 512) l++;

		if(j < 0) j = 0;
		if(j > 21) j = 21;
		if(j >= i-l) j = i-l-1;

		SDL_UpdateRect(sdl_screen,0,0,0,0);
		
		SDL_Delay(20);
	}
	
	char romname[256];
	sprintf(romname,"./roms/%s",roms[j+l]->d_name);
	
	char *p = romname+strlen(romname);
	for(;*p!='.'&&p!=romname;p--);
	
	if(p == romname) goto reread;
	p++;
	
	if(strcmp(p,"HEX") == 0 || strcmp(p,"hex") == 0 ||
	strcmp(p,"ihx") == 0 || strcmp(p,"ihx") == 0) {
		hexread(z80_memory,romname);
	} else {
		FILE *fp = fopen(romname,"rb");
		fread(z80_memory,1,0x8000,fp);
		fclose(fp);
	}
	
	
	for(i = 0; i < 512; i++) {
		free(roms[i]);
	}
	
	memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
	
	SDL_UpdateRect(sdl_screen,0,0,0,0);
}

void z80_vmreset(Z80_STATE *z80)
{
	memset(z80,0,sizeof(Z80_STATE));
	
	Z80Reset(z80);

	sec_tstates = 0;
	z80_tstates = 0;
	
	psg_sync = 0;
}

int main(int argc, char *argv[])
{
	SDL_Surface *sdl_screen;
	SDL_Event sdl_event;

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK);
	
	joy = SDL_JoystickOpen(0);

	sdl_screen = SDL_SetVideoMode(480,272,32,SDL_SWSURFACE);

	init_psg();

	z80_memory = (byte *)malloc(0x10000);
	memset(z80_memory,0xff,0x10000);

	Z80_STATE z80;
	
	memset(&z80,0,sizeof(Z80_STATE));
	
	Z80Reset(&z80);
	
	z80_vmreset(&z80);

	libgpu_init();
	
	memset(z80_memory,0xff,0x8000);
	
	chld_rom(sdl_screen);
	
	int s_frame = 0;
	int frame = 0;
	char fps_string[64] = "0.00 fps";
	
	int timer_base = SDL_GetTicks();

	while(!poll_event(&sdl_event)) {
		if((s_frame % (int)(FPS)) == 0) sec_tstates = z80_tstates;
		
		if(SDL_JoystickGetButton(joy,4) && SDL_JoystickGetButton(joy,5)) {
			reset_psg();
			
			memset(z80_memory,0xff,0x10000);
			
			chld_rom(sdl_screen);
			
			z80_vmreset(&z80);
			
			s_frame = 0;
			
			memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
			
			libgpu_puts(sdl_screen,0,272-8,0xffffff,"Please wait...");
			
			SDL_UpdateRect(sdl_screen,0,0,0,0);
			
			memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
			
			SDL_UpdateRect(sdl_screen,0,0,0,0);
		}
		
		if(SDL_JoystickGetButton(joy,0) && SDL_JoystickGetButton(joy,3)) {
			reset_psg();
			
			z80_vmreset(&z80);
			
			s_frame = 0;
			
			memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
			
			libgpu_puts(sdl_screen,0,272-8,0xffffff,"Please wait...");
			
			SDL_UpdateRect(sdl_screen,0,0,0,0);
			
			memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
			
			SDL_UpdateRect(sdl_screen,0,0,0,0);
		}
		z80_execute(&z80);
		adjustFPS();
		vdp_grefresh(sdl_screen);
		SDL_UpdateRect(sdl_screen,144,31,168,210);
		memset(sdl_screen->pixels,0,sdl_screen->pitch*8);
		libgpu_puts(sdl_screen,0,0,0xffffff,fps_string);
		SDL_UpdateRect(sdl_screen,0,0,64*6,8);
		s_frame++;
		frame++;
		
		if(SDL_GetTicks() - timer_base >= 1000) {
			sprintf(fps_string,"%.2f fps",(float)frame/((float)(SDL_GetTicks() - timer_base)/1000.0f));
			timer_base = SDL_GetTicks();
			frame = 0;
		}
	}

	SDL_JoystickClose(joy);

	SDL_Quit();

	return 0;
}