/*

gcc -std=gnu11 -Wall -Wextra $(pkg-config --cflags sdl2,SDL2_mixer,libmagic) -o audio.{out,c} $(pkg-config --libs sdl2,SDL2_mixer,libmagic) && ./audio.out

cscope $(pkg-config --cflags-only-I sdl2,SDL2_mixer,libmagic) -1 SDL_KEYDOWN audio.c
cscope $(pkg-config --cflags-only-I sdl2,SDL2_mixer,libmagic) -1 SDL_Event audio.c

https://lazyfoo.net/tutorials/SDL/21_sound_effects_and_music/index.php
cp -v tmp/soft-hitwhistle.wav 21_sound_effects_and_music/high.wav
ffmpeg -y -i tmp/Skyreach.mp3 21_sound_effects_and_music/beat.wav
pkgs="sdl2,SDL2_image,SDL2_ttf,SDL2_mixer"; g++ -std=c++11 -Wall -Wextra $(pkg-config --cflags "$pkgs") -o lazyfoo.out 21_sound_effects_and_music/21_sound_effects_and_music.cpp $(pkg-config --libs "$pkgs"); unset -v pkgs

*/

#include <assert.h>
#include <stdlib.h>
#include <magic.h>

// #include <SDL2/SDL_audio.h> // Complicated
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define RED   "\e[31m"
#define GREEN "\e[32m"
#define BROWN "\e[33m"
#define BLUE  "\e[34m"
#define RESET "\e[0m"

static SDL_Window *gWindow=NULL;
magic_t cookie=NULL;
static Mix_Music *gMusic=NULL;
static Mix_Chunk *eff_Whistle=NULL;
static Mix_Chunk *eff_Drum=NULL;

static void SDL_Init2(){
  // Can be called before SDL_Init()
  SDL_version c={};
  SDL_version l={};
  SDL_VERSION(&c);
  SDL_GetVersion(&l);
  printf("compiled against SDL2 %d.%d.%d\n",       c.major,c.minor,c.patch);
  printf("runtime          SDL2 %d.%d.%d rev %s\n",l.major,l.minor,l.patch,SDL_GetRevision());
  printf("%smatch"RESET"\n", (0==memcmp(&c,&l,sizeof(SDL_version))) ? GREEN : RED"mis" );
  assert(0==SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_VIDEO));
}

static void SDL_CreateWindow2(){
  SDL_CreateWindow("fahigkeit",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,320,240,SDL_WINDOW_SHOWN);
}

static void Mix_OpenAudio2(){
  SDL_version mc=(SDL_version){};
  SDL_MIXER_VERSION(&mc);
  const SDL_version *const ml=Mix_Linked_Version();
  printf("compiled against SDL2_mixer %d.%d.%d\n",mc.major ,mc.minor ,mc.patch );
  printf("runtime          SDL2_mixer %d.%d.%d\n",ml->major,ml->minor,ml->patch);
  printf("%smatch"RESET"\n", (0==memcmp(&mc,ml,sizeof(SDL_version))) ?  GREEN : RED"mis" );
  // const int frequency=MIX_DEFAULT_FREQUENCY // 22050
  const int frequency=44100; // CPU heavy
  static_assert(SDL_BYTEORDER==SDL_LIL_ENDIAN);
  static_assert(MIX_DEFAULT_FORMAT==AUDIO_S16LSB); // Signed 16-bit samples, in little-endian byte order
  const Uint16 format=MIX_DEFAULT_FORMAT;
  const int channels=2; // Stereo
  const int chunksize=2048; // 2 kilobytes // Too small => sound skip, too large => sound lag
  assert(0==Mix_OpenAudio(frequency,format,channels,chunksize));
  // This may or may not match the parameters you passed to Mix_OpenAudio
  int frequency2=-1;
  Uint16 format2=-1;
  int channels2=-1;
  assert(1==Mix_QuerySpec(&frequency2,&format2,&channels2));
  assert(frequency2==frequency);
  assert(format2==format);
  assert(channels2==channels);
}

static void magic_open2(){
  assert(NULL==cookie);
  cookie=magic_open(MAGIC_SYMLINK|MAGIC_MIME_TYPE|MAGIC_CHECK|MAGIC_NO_CHECK_COMPRESS|MAGIC_NO_CHECK_TAR);
  assert(cookie);
  assert(0==magic_load(cookie,"/usr/share/file/misc/magic.mgc"));
}

Mix_Chunk *Mix_LoadWAV2(char *file){
  assert(0==strcmp(magic_file(cookie,file),"audio/x-wav"));
  // magic_buffer();
  return Mix_LoadWAV(file);
}

static void Mix_LoadXXX2(){

  const char *music_path="tmp/Skyreach.mp3";
  assert(0==strcmp(magic_file(cookie,music_path),"audio/mpegapplication/octet-stream"));
  gMusic=Mix_LoadMUS(music_path);assert(gMusic);

  eff_Whistle=Mix_LoadWAV2("tmp/soft-hitwhistle.wav");assert(eff_Whistle);
  eff_Drum=Mix_LoadWAV2("tmp/drum-hitnormal.wav");assert(eff_Drum);

  static_assert(MIX_MAX_VOLUME==128);
  printf("vol %d\n",Mix_VolumeMusic(-1));
  Mix_VolumeMusic(96);
  printf("vol %d\n",Mix_VolumeMusic(-1));

}

static void help(){
  printf(BLUE);
  puts(" <space> - whistle");
  puts(" <lshft> - drum");
  puts("   <9>   - play");
  puts("   <0>   - stop");
  puts("   <Q>   - quit");
  printf(RESET);fflush(stdout);
}

static void Mix_PlayChannel2(Mix_Chunk *const eff){
  const int ch=Mix_PlayChannelTimed(-1, eff, 0/*1 loop*/, 333/*ms*/);
  printf("channel %d\n",ch);
  // assert(ch==0||ch==1);
}

static void Mix_PlayMusic2(){
  assert(0==Mix_PausedMusic());
  const int r=Mix_PlayingMusic();
  if(1==r){
    puts(BROWN"already playing"RESET);
    return;
  }
  assert(0==r);
  assert(0==Mix_PlayMusic(gMusic,1));
}

static void Mix_HaltMusic2(){
  assert(0==Mix_PausedMusic());
  const int r=Mix_PlayingMusic();
  if(0==r){
    puts(BROWN"already halted"RESET);
    return;
  }
  assert(1==r);
  assert(0==Mix_HaltMusic());
}

static void SDL_PollEvent2(){

  // void SDL_SetEventFilter(SDL_EventFilter filter,void *userdata);

  SDL_Event e={};

  // Frequent case goes first
  for(;;){

    const int r=SDL_PollEvent(&e);
    if(r==0)continue;
    assert(r==1);

    // -- Ignore noisy events - quiet
    // /usr/include/SDL2/SDL_events.h
    // SDL_WINDOWEVENT 0x200 512
    // SDL_KEYUP       0x301 769
    // SDL_TEXTINPUT   0x303 771
    // SDL_MOUSEMOTION 0x400 1024
    const SDL_EventType *ign=(const SDL_EventType []){SDL_WINDOWEVENT,SDL_KEYUP,SDL_TEXTINPUT,SDL_MOUSEMOTION,SDL_FIRSTEVENT};
    for(;*ign!=SDL_FIRSTEVENT;++ign)
      if(e.type==*ign)
        goto super_continue;

    // -- Ignore peaceful events - verbose
    if(e.type!=SDL_KEYDOWN){printf("event %u ingored\n",e.type);continue;}
    assert(e.key.type==SDL_KEYDOWN);

    switch(e.key.keysym.scancode){
    // case SDL_SCANCODE_KP_SPACE:
    //   assert(SDLK_KP_SPACE==e.key.keysym.sym);
    //   puts("key <Keypad Space> accepted");
    //   break;
    case SDL_SCANCODE_SPACE:                // https://wiki.libsdl.org/SDL_Scancode
      assert(SDLK_SPACE==e.key.keysym.sym); // https://wiki.libsdl.org/SDL_Keycode
      puts("key <Space> accepted - whistle");
      Mix_PlayChannel2(eff_Whistle);
      break;
    case SDL_SCANCODE_LSHIFT:
      assert(SDLK_LSHIFT==e.key.keysym.sym);
      puts("key <Left Shift> accepted - drum");
      Mix_PlayChannel2(eff_Drum);
      break;
    case SDL_SCANCODE_9:
      assert(SDLK_9==e.key.keysym.sym);
      puts("key <9> accepted - play");
      Mix_PlayMusic2();
      break;
    case SDL_SCANCODE_0:
      assert(SDLK_0==e.key.keysym.sym);
      puts("key <0> accepted - stop");
      Mix_HaltMusic2();
      break;
    case SDL_SCANCODE_Q:
      assert(SDLK_q==e.key.keysym.sym);
      puts("key <Q> accepted - quit");
      return;
      break;
    default:
      printf("key %u ingored\n",e.key.keysym.scancode);
      continue;
    }
    printf("%ums\n",e.key.timestamp);
    // printf("%u\n",e.key.windowID);
    assert(2==e.key.windowID);
    printf("%u\n",e.key.repeat);

    super_continue:;

  }

}

static void quit(){
  Mix_HaltMusic2();
  assert(eff_Whistle);Mix_FreeChunk(eff_Whistle);eff_Whistle=NULL;
  assert(eff_Drum);Mix_FreeChunk(eff_Drum);eff_Drum=NULL;
  assert(gMusic);Mix_FreeMusic(gMusic);gMusic=NULL;
  magic_close(cookie);cookie=NULL;
  Mix_CloseAudio();
  if(gWindow){SDL_DestroyWindow(gWindow);gWindow=NULL;}
  SDL_Quit();
}

int main(){

  puts("");

  SDL_Init2();
  puts("");

  SDL_CreateWindow2();

  Mix_OpenAudio2();
  puts("");

  magic_open2();

  Mix_LoadXXX2();
  puts("");

  help();
  puts("");

  SDL_PollEvent2();

  quit();

  return EXIT_SUCCESS;

}
