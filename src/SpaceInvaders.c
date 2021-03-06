#include <stdio.h>
#include "8080Core.h"
#include <SDL/SDL.h>

SDL_Surface * screen;
SDL_Event ev;

u8 dip1 = 0x01;
u8 dip2 = 0x00;

u16 shiftReg = 0x0000;
u16 shiftOff = 0x0000;

ramBank wram8080[4] =
{
	{ 0x0000,
	  0x1FFF,
	  0x0000,
	  FLAG_ROM,
	  NULL
	},
	{ 0x2000,
	  0x0400,
	  0x0000,
	  FLAG_USED,
	  NULL
	},
	{ 0x2400,
	  0x1C00,
	  0x0000,
	  FLAG_USED,
	  NULL
	},
	{ 0x4000,
	  0xFFFF,
	  0x2000,
	  FLAG_MIRROR,
	  NULL
	},	 
};

u8 spaceInvaders_portIn (int port)
{
	switch (port) {
		case 1:
			return dip1;
		case 2:
			return dip2;
		case 3:
			return (shiftReg << shiftOff) >> 8;
		default:
			while (1);
	}
		
	return 0x00;
}

void spaceInvaders_portOut (int port, u8 value)
{
	switch (port) {
		case 2:
			shiftOff = value; break;
		case 4:
			shiftReg = (shiftReg << 8) | value; break;
		default:
			break;
	}
}

void spaceInvaders_vblank ()
{	
	int vramPtr, b;

	SDL_LockSurface(screen);
	
	u8 *screenPtr = screen->pixels;

	for (vramPtr = 0; vramPtr < 0x4000 - 0x2400; vramPtr++) { 
		for (b=0;b<8;b++) {
			*screenPtr = ((readByte(0x2400 + vramPtr) >> b)&1) ? 0xFF : 0x00; 
			screenPtr++;
		}
	}
	
	SDL_UnlockSurface(screen);
	
	SDL_Flip(screen);
}

void spaceInvaders_update_input ()
{
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_KEYDOWN:
				switch(ev.key.keysym.sym) {
					case SDLK_LEFT:
						dip1 |= (1 << 5); break;
					case SDLK_RIGHT:
						dip1 |= (1 << 6); break;
					case SDLK_c:
						dip1 |= (1 << 0); break;
					case SDLK_x:
						dip1 |= (1 << 2); break;
					case SDLK_z:
						dip1 |= (1 << 4); break;						
					default:
						break;
				} break;
			case SDL_QUIT:
				exit(0);
				break;
			default:
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
		printf("Cannot initialize SDL\n");
		exit(0);
	}
	
	atexit(SDL_Quit);
	
	SDL_EnableKeyRepeat(0, 0);
	
	screen = SDL_SetVideoMode(256, 224, 8, SDL_DOUBLEBUF);
	
	SDL_WM_SetCaption("OMGALIENZATEMYLEM0N", NULL);

	if (!initalize8080()) {
		printf("Error while initializing the processor\n");
		exit(0);
	}
	
	e8080.portIn = spaceInvaders_portIn;
	e8080.portOut = spaceInvaders_portOut;
	
	while (!e8080.halt)
	{		
		emulate8080(28527);
		causeInt(8);		
		spaceInvaders_vblank();	
		emulate8080(4839);
		causeInt(16);
		spaceInvaders_update_input();
		SDL_Delay(15);		
	}
	
	return 1;
}
