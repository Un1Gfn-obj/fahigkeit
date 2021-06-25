/*

pkgs="libmagic,minizip,sdl2,SDL2_mixer" && gcc -std=gnu11 -Wall -Wextra $(pkg-config --cflags "$pkgs") -o fahigkeit.out audio.c effect.c unzip.c main.c $(pkg-config --libs "$pkgs") && ./fahigkeit.out

pkgs="libmagic,minizip,sdl2,SDL2_mixer" cscope $(pkg-config --cflags-only-I "$pkgs") -1 XXX main.c

https://lazyfoo.net/tutorials/SDL/21_sound_effects_and_music/index.php
cp -v tmp/soft-hitwhistle.wav 21_sound_effects_and_music/high.wav
ffmpeg -y -i tmp/Skyreach.mp3 21_sound_effects_and_music/beat.wav
pkgs="sdl2,SDL2_image,SDL2_ttf,SDL2_mixer"; g++ -std=c++11 -Wall -Wextra $(pkg-config --cflags "$pkgs") -o lazyfoo.out 21_sound_effects_and_music/21_sound_effects_and_music.cpp $(pkg-config --libs "$pkgs"); unset -v pkgs

*/

#include <assert.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h> // Mix_CloseAudio()

#include "./audio.h"
#include "./def.h"

static SDL_Window *gWindow=NULL;

// unzip.c
extern void unzip();

static void SDL_Init2(){
  // Can be called before SDL_Init()
  SDL_version c={};
  SDL_version l={};
  SDL_VERSION(&c);
  SDL_GetVersion(&l);
  printf("compiled against SDL2 %d.%d.%d\n",       c.major,c.minor,c.patch);
  printf("runtime          SDL2 %d.%d.%d rev %s\n",l.major,l.minor,l.patch,SDL_GetRevision());
  printf("%smatch" RESET "\n", (0==memcmp(&c,&l,sizeof(SDL_version))) ? GREEN : RED "mis" );
  assert(0==SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_VIDEO));
}

int main(){

  puts("");

  unzip();puts("");

  SDL_Init2();puts("");

  assert((gWindow=SDL_CreateWindow("fahigkeit",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,320,240,SDL_WINDOW_SHOWN)));

  Mix_OpenAudio2();puts("");

  Mix_LoadXXX();puts("");

  help();puts("");

  SDL_PollEvent2();

  Mix_FreeXXX();

  Mix_CloseAudio();

  assert(gWindow);SDL_DestroyWindow(gWindow);gWindow=NULL;

  SDL_Quit();

  return EXIT_SUCCESS;

}
