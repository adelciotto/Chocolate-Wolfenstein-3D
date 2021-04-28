//
// Created by Anthony Del Ciotto on 15/4/21.
//

#include "sdl_vl.h"
#include "log.h"
#include "wl_def.h"

#define PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888

SDL_Surface *g_screen = NULL;
SDL_Surface *g_indexedScreen = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *screenTexture = NULL;
static SDL_Texture *scaledTexture = NULL;

void SDL_VL_Init(const char *title, uint32_t width, uint32_t height, uint32_t scaledWidth, uint32_t scaledHeight,
                 bool fullscreen)
{
    // Create the SDL Window.
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scaledWidth, scaledHeight,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window)
    {
        LOG_Errorf("Unable to create SDL_Window %ix%i: %s", scaledWidth, scaledHeight, SDL_GetError());
        Quit("");
    }

    // Create the SDL Renderer.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        LOG_Errorf("Unable to create SDL_Renderer: %s", SDL_GetError());
        Quit("");
    }
    SDL_ShowCursor(SDL_DISABLE);

    // Create the indexed screen surface which the game will draw into using a color palette.
    g_indexedScreen = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    if (!g_indexedScreen)
    {
        LOG_Errorf("Unable to create indexed surface: %s", SDL_GetError());
        Quit("");
    }

    // Create the screen surface which will contain a 32bit ARGB version of the indexed screen.
    uint32_t rmask, gmask, bmask, amask;
    int bpp;
    SDL_PixelFormatEnumToMasks(PIXEL_FORMAT, &bpp, &rmask, &gmask, &bmask, &amask);
    g_screen = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
    if (!g_screen)
    {
        LOG_Errorf("Unable to create screen surface: %s", SDL_GetError());
        Quit("");
    }

    // Create the intermediate texture that we render the screen surface into.
    screenTexture = SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!screenTexture)
    {
        LOG_Errorf("Unable to create screen texture: %s", SDL_GetError());
        Quit("");
    }

    // Create the up-scaled texture that we render to the window.
    scaledTexture = SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_TARGET, scaledWidth, scaledHeight);
    if (!scaledTexture)
    {
        LOG_Errorf("Unable to create scaled texture: %s", SDL_GetError());
        Quit("");
    }

    SDL_RenderSetLogicalSize(renderer, scaledWidth, scaledHeight);

    if (fullscreen)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        LOG_Infof("SDL window set to fullscreen with size: %dx%d", w, h);
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    LOG_Infof("SDL renderer initialized. Driver: %s. Surface size: %dx%d. CRT Texture size: %dx%d", info.name, width,
              height, scaledWidth, scaledHeight);

    for (uint32_t i = 0; i < info.num_texture_formats; i++)
    {
        LOG_Infof("Supported Texture Format: %s", SDL_GetPixelFormatName(info.texture_formats[i]));
    }
}

void SDL_VL_Destroy()
{
    if (scaledTexture)
        SDL_DestroyTexture(scaledTexture);

    if (screenTexture)
        SDL_DestroyTexture(screenTexture);

    if (g_screen)
        SDL_FreeSurface(g_screen);

    if (g_indexedScreen)
        SDL_FreeSurface(g_indexedScreen);

    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    LOG_Infof("SDL window and renderer resources destroyed");
}

void SDL_VL_SetPaletteColors(SDL_Color *colors)
{
    SDL_SetPaletteColors(g_indexedScreen->format->palette, colors, 0, 256);
}

void SDL_VL_SetSurfacePalette(SDL_Surface *surface)
{
    SDL_SetSurfacePalette(surface, g_indexedScreen->format->palette);
}

void SDL_VL_BlitIndexedSurfaceToScreen()
{
    SDL_BlitSurface(g_indexedScreen, NULL, g_screen, NULL);
}

void SDL_VL_Present()
{
    SDL_UpdateTexture(screenTexture, NULL, g_screen->pixels, g_screen->pitch);
    SDL_RenderClear(renderer);

    // Render the intermediate texture into the up-scaled texture using 'nearest' integer scaling.
    // This up-scaled texture is an emulation of old CRT distortion that would stretch the image.
    SDL_SetRenderTarget(renderer, scaledTexture);
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);

    // Finally render this up-scaled texture to the window. SDL will decide on the scaling algorithm.
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, scaledTexture, NULL, NULL);

    SDL_RenderPresent(renderer);
}
