// This example is created in a single file to show that the general components
// of a paradigm are very simple. You most likely want to create individual
// files for your project to add some structure for a realistic application.

#include "../include/lsl_c.h" // the C API for LSL - get the include folder from the Releases at https://github.com/sccn/liblsl
#include "SDL2/SDL_render.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // SDL2_image is a separate library for images
#include <SDL2/SDL_ttf.h> // true type font rendering is also a separate library
#include <stdio.h>
#include <stdlib.h>

const char INLET_STREAM_NAME[] = "MyParadigmIncoming";
const char OUTLET_STREAM_NAME[] = "MyParadigmMarkers";

const char *const MARKER_STRINGS[] = {"Start", "End", "Left", "Right"};
const int NUM_MARKERS = sizeof(MARKER_STRINGS) / sizeof(MARKER_STRINGS[0]);

// Store a stimulus information for easy rendering
struct stimulus {
  SDL_Texture *texture;
  SDL_Rect rect;
  const char *text;
};

// Example from here:
// https://stackoverflow.com/questions/22886500/how-to-render-text-in-sdl2
void populate_texture_stimulus(SDL_Renderer *renderer, int x, int y,
                               const char *text, TTF_Font *font,
                               struct stimulus *stimulus) {

  printf("Creating texture for: %s\n", text);
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {205, 255, 205, 0};
  SDL_Rect *rect = &stimulus->rect;

  surface = TTF_RenderText_Solid(font, text, textColor);
  stimulus->texture = SDL_CreateTextureFromSurface(renderer, surface);
  stimulus->text = text;

  text_width = surface->w / 2; // on default the surface is the full window size
  text_height = surface->h / 2; // in a real app -> control the rect size
  SDL_FreeSurface(surface);
  // We off-set x and y to center of the screen
  rect->x = x + text_width / 2;
  rect->y = y + text_height / 2;
  rect->w = text_width;
  rect->h = text_height;
}

// For stimplicity, we work with many args, but in a real application
// you should consider creating a context struct to hold all the information
void destroy_context(SDL_Window *window, SDL_Renderer *renderer,
                     lsl_outlet outlet, lsl_inlet inlet) {

  if (window != NULL)
    SDL_DestroyWindow(window);
  if (renderer != NULL)
    SDL_DestroyRenderer(renderer);
  if (outlet != NULL)
    lsl_destroy_outlet(outlet);
  if (inlet != NULL)
    lsl_destroy_inlet(inlet);
  SDL_Quit();
}

int main(void) {
  // Some variables we use
  char *curr_marker;
  double timestamp;
  int errcode;
  int frame_time;
  float fps;
  int start_time = SDL_GetTicks();

  //-------------------------------------------------------------------------
  // init LSL, an `outlet` to write to and and `inlet` to listen to
  //-------------------------------------------------------------------------
  //    the outlet for markers sent from this program
  lsl_streaminfo out_info =
      lsl_create_streaminfo(OUTLET_STREAM_NAME, "Markers", 1,
                            LSL_IRREGULAR_RATE, cft_string, "myuid234457");
  lsl_outlet outlet = lsl_create_outlet(out_info, 0, 20);
  //    the inlet for markers processed by this program
  lsl_streaminfo in_info;

  // Note: This will only continue once this program finds the incoming stream
  // being present
  printf("Waiting for LSL Stream: %s...\n", INLET_STREAM_NAME);
  lsl_resolve_byprop(&in_info, 1, "name", INLET_STREAM_NAME, 1, LSL_FOREVER);
  lsl_inlet inlet = lsl_create_inlet(in_info, 20, LSL_NO_PREFERENCE, 1);

  //-------------------------------------------------------------------------
  // Initialize SDL - Window/Graphics
  //-------------------------------------------------------------------------
  SDL_Event event;

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  IMG_Init(IMG_INIT_PNG);
  SDL_Window *win =
      SDL_CreateWindow("Hello World!", 100, 100, 620, 387, SDL_WINDOW_SHOWN);
  if (win == NULL) {
    fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  // The renderer with VSynch will cap the framerate to the devices max, remove
  // it for higher frame rates
  SDL_Renderer *ren = SDL_CreateRenderer(
      win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  // win, -1, SDL_RENDERER_ACCELERATED);
  if (ren == NULL) {
    fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
    destroy_context(win, NULL, outlet, inlet);
    return EXIT_FAILURE;
  }

  //-------------------------------------------------------------------------
  // Create textures -> they will be loaded into the GPU VRAM for faster
  //-------------------------------------------------------------------------
  // rendering
  SDL_Texture *tex = IMG_LoadTexture(ren, "../assets/cat_eeg.png");
  if (tex == NULL) {
    fprintf(stderr, "IMG_LoadTexture Error: %s\n", SDL_GetError());
    destroy_context(win, ren, outlet, inlet);
    return EXIT_FAILURE;
  }

  // Store information as stimulus objects including textures
  //   Font for rendering text
  TTF_Init();
  TTF_Font *font =
      TTF_OpenFont("../assets/FiraCodeNerdFontMono-Regular.ttf",
                   256); // choose a very large font size for better resolution
  if (!font) {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    destroy_context(win, ren, outlet, inlet);
    return EXIT_FAILURE;
    // handle error
  }

  // Pre render the text
  struct stimulus stimuli[NUM_MARKERS];
  for (size_t i = 0; i < NUM_MARKERS; i++) {
    printf("Initializing stimulus: %s\n", MARKER_STRINGS[i]);
    populate_texture_stimulus(ren, 0, 0, MARKER_STRINGS[i], font, &stimuli[i]);
  }

  //-------------------------------------------------------------------------
  // Start the main loop
  //-------------------------------------------------------------------------

  // Write to oulet example
  const char *start_msg = "StartingMyParadigm";
  lsl_push_sample_str(outlet, &start_msg);

  int i = 0;
  while (1) {
    // Check inlet if relevant marker present
    timestamp = lsl_pull_sample_str(inlet, &curr_marker, 1, 0, &errcode);

    // Iterate over the predefined markers
    int j = (i / 20) % NUM_MARKERS;
    struct stimulus *curr_stim = &stimuli[j];

    SDL_RenderClear(ren);

    // Here we react to different markers we receive via LSL inlet
    if (strcmp(curr_marker, "cat") == 0) {
      SDL_RenderCopy(ren, tex, NULL, NULL);
      SDL_RenderPresent(ren);
      SDL_Delay(2000);
    } else {
      SDL_RenderCopy(ren, curr_stim->texture, NULL, &curr_stim->rect);
    };

    SDL_RenderPresent(ren);

    // If the Window is closed; Ctrl + C will also create a SDL_QUIT
    if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
      break;

    i = i + 1;
    frame_time = SDL_GetTicks() - start_time;
    fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;
    printf("Frame time: %d ms, FPS: %.1f\n", frame_time, fps);
    start_time = SDL_GetTicks();
  }

  // clean up
  for (size_t i = 0; i < NUM_MARKERS; i++) {
    printf("Freeing up texture for: %s\n", stimuli[i].text);
    SDL_DestroyTexture(stimuli[i].texture);
  }
  SDL_DestroyTexture(tex);
  destroy_context(win, ren, outlet, inlet);

  return EXIT_SUCCESS;
}
