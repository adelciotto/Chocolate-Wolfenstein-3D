//
// Created by Anthony Del Ciotto on 15/4/21.
//

#ifndef SDL_VL_H
#define SDL_VL_H

#include "SDL.h"

extern SDL_Surface *g_rgbaSurface;
extern SDL_Surface *g_paletteSurface;

void SDL_VL_Init(const char *title, int originalWidth, int originalHeight, bool fullscreen);
void SDL_VL_Destroy();

void SDL_VL_SetPaletteColors(SDL_Color *colors);
void SDL_VL_SetSurfacePalette(SDL_Surface *surface);

void SDL_VL_BlitIndexedSurfaceToScreen();
void SDL_VL_Present();

#endif // SDL_VL_H
