/*
 * Nikwi Deluxe
 * Copyright (C) 2005-2012 Kostas Michalopoulos
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Kostas Michalopoulos <badsector@runtimelegend.com>
 */

/*
** Nikwi Engine - GFX
*/

#include "nikwi.h"

//#define HALF_SIZED_SCREEN

SDL_Window	*window = NULL;
SDL_Renderer	*renderer = NULL;
SDL_Surface	*screen = NULL;
SDL_Texture	*sdlTexture = NULL;
bool		fullscreen = false;

static SDL_Joystick	*joy = NULL;

SDL_Surface *createSurface(int width, int height, bool colorKey)
{
	SDL_Surface	*surf;
	Uint32		rm, gm, bm;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	#error makeit
	rm = 0xFF000000;
	gm = 0x00FF0000;
	bm = 0x0000FF00;
	#else
	rm = 0x0000001F;
	gm = 0x000007E0;
	bm = 0x0000F800;
	#endif
	
	if (screen && screen->format)
	{
		rm = screen->format->Rmask;
		gm = screen->format->Gmask;
		bm = screen->format->Bmask;
	}

	surf = SDL_CreateRGBSurface(0,
		width, height, 32, rm, gm, bm, 0);
	if (!surf)
		return NULL;
		

	if (colorKey)
	{
		SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format,
			248, 0, 248));
	}
	
	return surf;
}

SDL_Surface *loadImage(String file)
{
	uint		len;
	unsigned char	*data = (unsigned char*)getData(file, len);
	unsigned short	width;
	unsigned short	height;
	unsigned short	*pixels;
	Uint32	*spixels;
	SDL_Surface	*surf;
	if (!data)
	{
		return NULL;
	}
	pixels = (unsigned short*)&data[8];
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	#error redo
	width = (data[0] << 8)|data[1];
	height = (data[2] << 8)|data[3];
	#else
	width = *((unsigned short*)(data + 4));
	height = *((unsigned short*)(data + 6));
	#endif
	
	surf = createSurface(width, height);
	if (!surf)
	{
		free(data);
		return NULL;
	}
	
	if (SDL_MUSTLOCK(surf))
		SDL_LockSurface(surf);
	uint	index = 0;
	for (uint h=0;h<height;h++)
	{
		spixels = &((Uint32*)surf->pixels)[h*surf->pitch/4];
		for (uint w=0;w<width;w++)
		{
			/*
			int	rv = pixels[index++];
			int	gv = pixels[index++];
			int	bv = pixels[index++];
			*/
			unsigned int	pixel = pixels[index++];
			int		rv, gv, bv;
			
			bv = (pixel&31) << 3;
			gv = ((pixel >> 5)&63) << 2;
			rv = ((pixel >> 11)&31) << 3;
			*(spixels++) = SDL_MapRGB(surf->format, rv, gv, bv);
/*			*(spixels++) = rv;
			*(spixels++) = gv;
			*(spixels++) = bv;
			*(spixels++) = 0;*/
		}
	}
	if (SDL_MUSTLOCK(surf))
		SDL_UnlockSurface(surf);
	
	free(data);
	
	return surf;
}

void drawLine(int x1, int y1, int x2, int y2, int color)
{
	Uint32 *pixels = (Uint32*)screen->pixels;
	float	len = hypot(x2 - x1, y2 - y1);
	float	x, y, dx, dy;
	
	dx = (x2 - x1)/len;
	dy = (y2 - y1)/len;
	
	x = x1;
	y = y1;
	
	for (int i=0;i<(int)len;i++,x+=dx,y+=dy)
		if (x >= 0 && x <= 639 && y >= 0 && y <= 479)
			pixels[(int)y*640 + (int)x] = color;
}

void drawBox(int x1, int y1, int x2, int y2, int color)
{
	Uint32 *pixels = (Uint32*)screen->pixels;
	for (int x=x1;x<x2;x++)
	{
		if (x < 0 || x > 639)
			continue;
		if (y1 >= 0 && y1 < 480)
			pixels[y1*640 + x] = color;
		if (y2 >= 0 && y2 < 480)
			pixels[y2*640 + x] = color;
	}
	for (int y=y1;y<y2;y++)
	{
		if (y < 0 || y > 479)
			continue;
		if (x1 >= 0 && x1 < 640)
			pixels[y*640 + x1] = color;
		if (x2 >= 0 && x2 < 640)
			pixels[y*640 + x2] = color;
	}
}

bool initGfx(String winCaption)
{
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK);
	int width = 640;
	int height = 480;
	#ifdef HALF_SIZED_SCREEN
	width = 320;
	height = 240;
	#endif

	window = SDL_CreateWindow(winCaption,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height,
		fullscreen ? SDL_WINDOW_FULLSCREEN : 0);

	renderer = SDL_CreateRenderer(window, -1, 0);

	screen = SDL_CreateRGBSurface(0, 640, 480, 32,
			0x00FF0000,
			0x0000FF00,
			0x000000FF,
			0xFF000000);

    sdlTexture = SDL_CreateTexture(renderer,
				SDL_PIXELFORMAT_RGB888,
				SDL_TEXTUREACCESS_STREAMING,
				640, 480);
	
	if (!window || !renderer || !screen || !sdlTexture)
		return false;
	SDL_ShowCursor(false);
	
	joy = SDL_JoystickOpen(0);
	
	return true;
}

void shutdownGfx()
{
	if (joy)
		SDL_JoystickClose(joy);
	joy = NULL;
	SDL_Quit();
}

void updateSystemScreen()
{
	SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

