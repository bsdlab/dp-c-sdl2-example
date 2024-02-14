// Example taken from here:
// https://gamedev.stackexchange.com/questions/59078/sdl-function-for-loading-pngs
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(void) {
  SDL_Event event;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *texture = NULL;
  SDL_Window *window = NULL;

  // Initialize SDL
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(500, 500, 0, &window, &renderer);
  IMG_Init(IMG_INIT_PNG);

  // Note that this loads the image into a texture, i.e. GPU VRAM instead of
  // regular RAM which should lead to greatly increased rendering speed
  // https://stackoverflow.com/questions/21392755/difference-between-surface-and-texture-sdl-general
  texture = IMG_LoadTexture(renderer, "../assets/cat_eeg.png");

  // Main loop
  while (1) {
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
      break;
  }

  // Cleanup
  SDL_DestroyTexture(texture);
  IMG_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
