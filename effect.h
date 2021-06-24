#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

typedef struct {
  const char *filename;
  // unsigned char *buf;
  Mix_Chunk *chunk;
  SDL_Scancode scancode;
  SDL_Keycode sym;
  const char *prompt;
} Effect;

extern Effect effects[];
