//
//  crt.c
//  Chocolate Wolfenstein 3D
//
//  Created by fabien sanglard on 2014-08-26.
//
//

#include "crt.h"
#include "id_vl.h"

static int width;
static int height;

static void CRT_Screenshot();

void CRT_Init(int _width)
{
    width  = _width;
    height = _width * 3.0/4.0;
}

void CRT_DAC(void)
{
    SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);

    SDL_Rect dest = {.x = 0, .y = 0, .w = width, .h = height};
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &dest);
    SDL_RenderPresent(sdlRenderer);

    const uint8_t *keyState = SDL_GetKeyboardState(NULL);
    static int wasPressed = 0;
    if (keyState[SDL_SCANCODE_I])
    {
        if (!wasPressed)
        {
            wasPressed = 1;
            CRT_Screenshot();
        }
    }
    else
    {
        wasPressed = 0;
    }
}

void CRT_Screenshot()
{
    const char* filename = "screenshot.tga";

    int pitch = width*4;
    size_t size = pitch*height;
    uint8_t *pixels = (uint8_t*) malloc(size);
    if (!pixels)
        return;

    SDL_RenderReadPixels(sdlRenderer, NULL, SDL_PIXELFORMAT_ARGB8888, pixels, pitch);

    //Now the file creation
    FILE *filePtr = fopen(filename, "wb");
    if (!filePtr)
        return;

    uint8_t TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};
    uint8_t header[6] = {width%256, width/256, height%256, height/256, 32, 0 | 1 << 5};

    // We write the headers
    fwrite(TGAheader, sizeof(uint8_t),12, filePtr);
    fwrite(header,	sizeof(uint8_t),6, filePtr);
    // And finally our image data
    fwrite(pixels,	sizeof(uint8_t), pitch*height, filePtr);
    fclose(filePtr);

    printf("Screenshot taken to screenshot.tga");
}
