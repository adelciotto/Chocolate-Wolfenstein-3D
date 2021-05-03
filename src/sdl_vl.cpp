//
// Created by Anthony Del Ciotto on 15/4/21.
//

#include "sdl_vl.h"
#include "log.h"
#include "wl_def.h"

#define PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888
#define CRT_SCREEN_WIDTH 640
#define CRT_SCREEN_HEIGHT 480

// 4k resolution 4096x2160 is 8,847,360
#define MAX_SCREEN_TEXTURE_PIXELS 8847360

SDL_Surface *g_rgbaSurface = NULL;
SDL_Surface *g_paletteSurface = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_RendererInfo rendererInfo;
static SDL_Texture *intermediateTexture = NULL;
static SDL_Texture *crtScaleTexture = NULL;
static SDL_Texture *screenTexture = NULL;

static void getScreenTextureUpscale(int *widthUpscale, int *heightUpscale);

void SDL_VL_Init(const char *title, int originalWidth, int originalHeight, bool fullscreen)
{
    // Create the SDL Window.
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CRT_SCREEN_WIDTH,
                              CRT_SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window)
    {
        Quit("Unable to create SDL_Window %ix%i: %s", CRT_SCREEN_WIDTH, CRT_SCREEN_HEIGHT, SDL_GetError());
    }

    // Create the SDL Renderer.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        Quit("Unable to create SDL_Renderer: %s", SDL_GetError());
    }

    if (SDL_GetRendererInfo(renderer, &rendererInfo) != 0)
    {
        Quit("Unable to get SDL_RenderInfo: %s", SDL_GetError());
    }

    SDL_ShowCursor(SDL_DISABLE);
    SDL_RenderSetLogicalSize(renderer, CRT_SCREEN_WIDTH, CRT_SCREEN_HEIGHT);

    // Create the indexed screen surface which the game will draw into using a color palette.
    g_paletteSurface = SDL_CreateRGBSurface(0, originalWidth, originalHeight, 8, 0, 0, 0, 0);
    if (!g_paletteSurface)
    {
        Quit("Unable to create palette surface: %s", SDL_GetError());
    }

    // Create the screen surface which will contain a 32bit ARGB version of the indexed screen.
    uint32_t rmask, gmask, bmask, amask;
    int bpp;
    SDL_PixelFormatEnumToMasks(PIXEL_FORMAT, &bpp, &rmask, &gmask, &bmask, &amask);
    g_rgbaSurface = SDL_CreateRGBSurface(0, originalWidth, originalHeight, bpp, rmask, gmask, bmask, amask);
    if (!g_rgbaSurface)
    {
        Quit("Unable to create rgba surface: %s", SDL_GetError());
    }

    // Create the intermediate texture that we render the rgba surface into.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    intermediateTexture =
        SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_STREAMING, originalWidth, originalHeight);
    if (!intermediateTexture)
    {
        Quit("Unable to create intermediate texture: %s", SDL_GetError());
    }

    // Create the CRT aspect ratio corrected texture that we render the original texture to.
    // The original texture is scaled vertically to emulate CRT distortion (640x400 -> 640x480).
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    crtScaleTexture =
        SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_TARGET, CRT_SCREEN_WIDTH, CRT_SCREEN_HEIGHT);
    if (!crtScaleTexture)
    {
        Quit("Unable to create CRT scaled texture: %s", SDL_GetError());
    }

    if (fullscreen)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    // Create the final screen texture that is an integer scaled up version of the CRT texture. The scale is determined
    // by the window size. If the window aspect ratio differs from the CRT ratio (640 / 480) then the screen texture
    // will be scaled down to fit on the screen using 'linear' smooth scaling.
    //
    // For example, on a window of size 1920x1080, the screen texture would be size 1920x1440 (scale of 3).
    // This would then be scaled down smoothly to fit into the window.
    // Credit to crispy-doom for this strategy: https://github.com/fabiangreffrath/crispy-doom/blob/master/src/i_video.c
    int widthUpscale, heightUpscale;
    getScreenTextureUpscale(&widthUpscale, &heightUpscale);
    int screenTextureW = CRT_SCREEN_WIDTH * widthUpscale;
    int screenTextureH = CRT_SCREEN_HEIGHT * heightUpscale;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    screenTexture = SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_TARGET, screenTextureW, screenTextureH);
    if (!screenTexture)
    {
        Quit("Unable to create screen texture: %s", SDL_GetError());
    }

    LOG_Infof("SDL renderer initialized with driver: %s", rendererInfo.name);
    LOG_Infof(
        "Surface size: %dx%d, Pixel format: %s, CRT texture size: %dx%d, Screen texture [size: %dx%d, upscale: %dx%d]",
        originalWidth, originalHeight, SDL_GetPixelFormatName(PIXEL_FORMAT), CRT_SCREEN_WIDTH, CRT_SCREEN_HEIGHT,
        screenTextureW, screenTextureH, widthUpscale, heightUpscale);
}

void SDL_VL_Destroy()
{
    if (screenTexture)
        SDL_DestroyTexture(screenTexture);

    if (crtScaleTexture)
        SDL_DestroyTexture(crtScaleTexture);

    if (intermediateTexture)
        SDL_DestroyTexture(intermediateTexture);

    if (g_rgbaSurface)
        SDL_FreeSurface(g_rgbaSurface);

    if (g_paletteSurface)
        SDL_FreeSurface(g_paletteSurface);

    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    LOG_Infof("SDL window and renderer resources destroyed");
}

void SDL_VL_SetPaletteColors(SDL_Color *colors)
{
    SDL_SetPaletteColors(g_paletteSurface->format->palette, colors, 0, 256);
}

void SDL_VL_SetSurfacePalette(SDL_Surface *surface)
{
    SDL_SetSurfacePalette(surface, g_paletteSurface->format->palette);
}

void SDL_VL_BlitIndexedSurfaceToScreen()
{
    SDL_BlitSurface(g_paletteSurface, NULL, g_rgbaSurface, NULL);
}

void SDL_VL_Present()
{
    SDL_UpdateTexture(intermediateTexture, NULL, g_rgbaSurface->pixels, g_rgbaSurface->pitch);
    SDL_RenderClear(renderer);

    // Render the intermediate texture into the CRT texture using 'nearest' scaling.
    SDL_SetRenderTarget(renderer, crtScaleTexture);
    SDL_RenderCopy(renderer, intermediateTexture, NULL, NULL);

    // Render the CRT texture to the screen texture using 'nearest' scaling.
    SDL_SetRenderTarget(renderer, screenTexture);
    SDL_RenderCopy(renderer, crtScaleTexture, NULL, NULL);

    // Render the screen texture to the window using 'linear' scaling.
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);

    SDL_RenderPresent(renderer);
}

static void limitScreenTextureSize(int *widthUpscale, int *heightUpscale)
{
    int maxTextureWidth = rendererInfo.max_texture_width;
    int maxTextureHeight = rendererInfo.max_texture_height;

    while (*widthUpscale * CRT_SCREEN_WIDTH > maxTextureWidth)
    {
        --*widthUpscale;
    }
    while (*heightUpscale * CRT_SCREEN_HEIGHT > maxTextureHeight)
    {
        --*heightUpscale;
    }

    if ((*widthUpscale < 1 && maxTextureWidth > 0) || (*heightUpscale < 1 && maxTextureHeight > 0))
    {
        Quit("Unable to create a texture big enough for the whole screen! Maximum texture size %dx%d", maxTextureWidth,
             maxTextureWidth);
    }

    // We limit the amount of texture memory used for the screen texture,
    // since beyond a certain point there are diminishing returns. Also,
    // depending on the hardware there may be performance problems with very huge textures.
    while (*widthUpscale * *heightUpscale * CRT_SCREEN_WIDTH * CRT_SCREEN_HEIGHT > MAX_SCREEN_TEXTURE_PIXELS)
    {
        if (*widthUpscale > *heightUpscale)
        {
            --*widthUpscale;
        }
        else
        {
            --*heightUpscale;
        }
    }
}

void getScreenTextureUpscale(int *widthUpscale, int *heightUpscale)
{
    int w, h;
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
    {
        Quit("Failed to get the renderer output size: %s", SDL_GetError());
    }

    if (w > h)
    {
        // Wide window.
        w = h * (CRT_SCREEN_WIDTH / CRT_SCREEN_HEIGHT);
    }
    else
    {
        // Tall window.
        h = w * (CRT_SCREEN_HEIGHT / CRT_SCREEN_WIDTH);
    }

    *widthUpscale = (w + CRT_SCREEN_WIDTH - 1) / CRT_SCREEN_WIDTH;
    *heightUpscale = (h + CRT_SCREEN_HEIGHT - 1) / CRT_SCREEN_HEIGHT;

    if (*widthUpscale < 1)
        *widthUpscale = 1;
    if (*heightUpscale < 1)
        *heightUpscale = 1;

    limitScreenTextureSize(widthUpscale, heightUpscale);
}
