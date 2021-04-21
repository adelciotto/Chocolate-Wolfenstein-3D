//
// Created by Anthony Del Ciotto on 15/4/21.
//

#include "sdl_vl.h"
#include "log.h"

#define PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888

SDL_Surface *g_screen = NULL;
SDL_Surface *g_indexedScreen = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *screenTexture = NULL;
static SDL_Texture *scaledTexture = NULL;
static SDL_Palette *colorPalette = NULL;

void SDL_VL_Init(const char *title, uint32_t width, uint32_t height, uint32_t scaledWidth, uint32_t scaledHeight,
                 bool fullscreen)
{
    uint32_t rmask, gmask, bmask, amask;

    // Create the SDL Window.
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scaledWidth, scaledHeight,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window)
    {
        LOG_Errorf("Unable to create SDL_Window %ix%i: %s", scaledWidth, scaledHeight, SDL_GetError());
        goto error;
    }

    if (fullscreen)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        LOG_Infof("SDL window set to fullscreen with size: %dx%d", w, h);
    }

    // Create the SDL Renderer.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        LOG_Errorf("Unable to create SDL_Renderer: %s", SDL_GetError());
        goto error;
    }
    SDL_RenderSetLogicalSize(renderer, scaledWidth, scaledHeight);
    SDL_ShowCursor(SDL_DISABLE);

    // Create the color palette.
    colorPalette = SDL_AllocPalette(256);
    if (!colorPalette)
    {
        LOG_Errorf("Unable to allocate SDL_Palette: %s", SDL_GetError());
        goto error;
    }

    // Create the indexed screen surface which the game will draw into using a color palette.
    g_indexedScreen = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    if (!g_indexedScreen)
    {
        LOG_Errorf("Unable to create indexed surface: %s", SDL_GetError());
        goto error;
    }
    SDL_SetSurfacePalette(g_indexedScreen, colorPalette);

    // Create the screen surface which will contain a 32bit ARGB version of the indexed screen.
    rmask = 0x00ff0000;
    gmask = 0x0000ff00;
    bmask = 0x000000ff;
    amask = 0xff000000;
    g_screen = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
    if (!g_screen)
    {
        LOG_Errorf("Unable to create screen surface: %s", SDL_GetError());
        goto error;
    }

    // Create the intermediate texture that we render the screen surface into.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    screenTexture = SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!screenTexture)
    {
        LOG_Errorf("Unable to create screen texture: %s", SDL_GetError());
        exit(1);
    }

    // Create the up-scaled texture that we render to the window. We use 'linear' scaling here because depending on the
    // window size, the texture may need to be scaled by a non-integer factor.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    scaledTexture = SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_TARGET, scaledWidth, scaledHeight);
    if (!scaledTexture)
    {
        LOG_Errorf("Unable to create scaled texture: %s", SDL_GetError());
        goto error;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    LOG_Infof("SDL renderer initialized. Driver: %s. Surface size: %dx%d. CRT Texture size: %dx%d", info.name, width,
              height, scaledWidth, scaledHeight);

    return;

error:
    SDL_VL_Destroy();
    exit(1);
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

    if (colorPalette)
        SDL_FreePalette(colorPalette);

    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    LOG_Infof("SDL window and renderer resources destroyed");
}

void SDL_VL_SetPaletteColors(SDL_Color *colors)
{
    SDL_SetPaletteColors(colorPalette, colors, 0, 256);
}

void SDL_VL_SetSurfacePalette(SDL_Surface *surface)
{
    SDL_SetSurfacePalette(surface, colorPalette);
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

    // Finally render this up-scaled texture to the window using 'linear' scaling.
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, scaledTexture, NULL, NULL);

    SDL_RenderPresent(renderer);
}
