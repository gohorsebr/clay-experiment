#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_renderer_sdl2.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// meh! move this to makefile OR change to .h
#include "layout.c"

SDL_Surface *sample_image;

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

typedef struct ResizeRenderData_ {
    SDL_Window *window;
    int window_width;
    int window_height;
    ClayVideoDemo_Data demo_data;
    SDL_Renderer *renderer;
    SDL2_Font *fonts;
} ResizeRenderData;

int handle_resize(void *user_data, SDL_Event *event) {
    ResizeRenderData *data = user_data;
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_EXPOSED) {
        SDL_GetWindowSize(data->window, &data->window_width, &data->window_height);
        Clay_SetLayoutDimensions((Clay_Dimensions){(float)data->window_width, (float)data->window_height});

        Clay_RenderCommandArray renderCommands = ClayVideoDemo_CreateLayout(&data->demo_data);

        SDL_SetRenderDrawColor(data->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(data->renderer);

        Clay_SDL2_Render(data->renderer, renderCommands, data->fonts);
        SDL_RenderPresent(data->renderer);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // ------------------------------------------------------------------------
    // Init SDL, TTF, IMG, load font
    // ------------------------------------------------------------------------
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error: could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "Error: could not initialize TTF: %s\n", TTF_GetError());
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        fprintf(stderr, "Error: could not initialize IMG: %s\n", IMG_GetError());
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("resources/Roboto-Regular.ttf", 18);
    if (!font) {
        fprintf(stderr, "Error: could not load font: %s\n", TTF_GetError());
        return 1;
    }

    SDL2_Font fonts[1] = {};

    fonts[FONT_ID_BODY_16] = (SDL2_Font){
        .fontId = FONT_ID_BODY_16,
        .font = font,
    };

    sample_image = IMG_Load("resources/sample.png");

    // ------------------------------------------------------------------------
    // Create Window & Renderer
    // ------------------------------------------------------------------------
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl"); // for antialiasing
    window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // for antialiasing

    bool enableVsync = false;
    if (enableVsync) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); //"SDL_RENDERER_ACCELERATED" is for antialiasing
    } else {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // for alpha blending

    // ------------------------------------------------------------------------
    // Allocate memory for clay library
    // ------------------------------------------------------------------------
    uint64_t total_memory_size = Clay_MinMemorySize();
    Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(total_memory_size, malloc(total_memory_size));

    int window_width = 0;
    int window_height = 0;
    SDL_GetWindowSize(window, &window_width, &window_height);
    Clay_Initialize(clay_memory, (Clay_Dimensions){(float)window_width, (float)window_height}, (Clay_ErrorHandler){HandleClayErrors});

    Clay_SetMeasureTextFunction(SDL2_MeasureText, &fonts);

    // debug mode
    // Clay_SetDebugModeEnabled(true);

    Uint64 tick_now = SDL_GetPerformanceCounter();
    Uint64 tick_last = 0;
    double delta_time = 0;
    ClayVideoDemo_Data demo_data = ClayVideoDemo_Initialize();

    // on resize event, call resizeRendering
    ResizeRenderData user_data = {
        window,        // SDL_Window*
        window_width,  // int
        window_height, // int
        demo_data,     // CustomShit
        renderer,      // SDL_Renderer*
        fonts          // SDL2_Font[1]
    };
    // add an event watcher that will render the screen while youre dragging the window to different sizes
    SDL_AddEventWatch(handle_resize, &user_data);

    // ------------------------------------------------------------------------
    // Main loop
    // ------------------------------------------------------------------------
    bool running = true;
    while (running) {
        Clay_Vector2 scroll_delta = {};
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                running=false;
            }
            case SDL_MOUSEWHEEL: {
                scroll_delta.x = event.wheel.x;
                scroll_delta.y = event.wheel.y;
                break;
            }
            }
        }
        tick_last = tick_now;
        tick_now = SDL_GetPerformanceCounter();
        delta_time = (double)((tick_now - tick_last) * 1000 / (double)SDL_GetPerformanceFrequency());
        printf("%f\n", delta_time);

        // update mouse position & scroll
        int mouse_x = 0;
        int mouse_y = 0;
        Uint32 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
        Clay_Vector2 mouse_position = (Clay_Vector2){(float)mouse_x, (float)mouse_y};
        Clay_SetPointerState(mouse_position, mouse_state & SDL_BUTTON(1));
        Clay_UpdateScrollContainers(true /* enable drag scroll */, scroll_delta, delta_time);
 
        // update window size
        SDL_GetWindowSize(window, &window_width, &window_height);
        Clay_SetLayoutDimensions((Clay_Dimensions){(float)window_width, (float)window_height});

        // create layout
        Clay_RenderCommandArray renderCommands = ClayVideoDemo_CreateLayout(&demo_data);

        // clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // render current frame
        Clay_SDL2_Render(renderer, renderCommands, fonts);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    // cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
