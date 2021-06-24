#include "./effect.h"

Effect effects[]={
  {"soft-hitwhistle.wav",NULL,0,NULL,SDL_SCANCODE_SPACE,SDLK_SPACE,"key <Space> accepted - whistle"},
  {"drum-hitnormal.wav",NULL,0,NULL,SDL_SCANCODE_LSHIFT,SDLK_LSHIFT,"key <Left Shift> accepted - drum"},
  {NULL,NULL,0,NULL,0,0,NULL}
};
