#include "paradigm.h"
#include "server.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h> // Using SDL threads as we do not have pthreads on Windows
#include <stdlib.h>

int main(void) {
  SDL_Thread *server_thread;

  // Run the server in a separate thread to allow SDLs rendering in main thread
  printf("Starting Server Thread");
  server_thread = SDL_CreateThread(run_server, "ServerThread", NULL);
  SDL_Delay(2000);

  // Communication between the server and this main thread is realized via the
  // `paradigm_stop_event` which is defined in paradigm.c
  // 0: Run paradigm, 1: stop paradigm, -1: close server
  paradigm_stop_event = 1;
  while (paradigm_stop_event != -1) { // if -1 was set, we close down
    SDL_Delay(1000);
    // Start was set

    if (paradigm_stop_event == 0) {
      run_paradigm();
    }
  }

  SDL_WaitThread(server_thread, NULL);

  return EXIT_SUCCESS;
}
