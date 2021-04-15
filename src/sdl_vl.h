//
// Created by Anthony Del Ciotto on 15/4/21.
//

#ifndef SDL_VL_H
#define SDL_VL_H

#include "SDL.h"

extern SDL_Surface *screen;
extern SDL_Surface *indexedScreen;

void SDL_VL_Init(const char *title, uint32_t width, uint32_t height, uint32_t scaledWidth, uint32_t scaledHeight,
                 int bpp, bool fullscreen);
void SDL_VL_Destroy();

void SDL_VL_SetPaletteColors(SDL_Color *colors);
void SDL_VL_SetSurfacePalette(SDL_Surface *surface);

void SDL_VL_BlitIndexedSurfaceToScreen();
void SDL_VL_Present();

#endif // SDL_VL_H
