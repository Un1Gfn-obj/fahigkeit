#include <assert.h>
#include <stdlib.h>
#include <magic.h>

// #include <SDL2/SDL_audio.h> // Complicated
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "./def.h"
#include "./effect.h"

static magic_t cookie=NULL;
static Mix_Music *gMusic=NULL;

void Mix_OpenAudio2(){
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

static Mix_Chunk *Mix_LoadWAV2(const char *const file){
  assert(0==strcmp(magic_file(cookie,file),"audio/x-wav"));
  // magic_buffer();
  return Mix_LoadWAV(file);
}

void Mix_LoadXXX(){

  // Init libmagic
  assert(NULL==cookie);
  cookie=magic_open(MAGIC_SYMLINK|MAGIC_MIME_TYPE|MAGIC_CHECK|MAGIC_NO_CHECK_COMPRESS|MAGIC_NO_CHECK_TAR);
  assert(cookie);
  assert(0==magic_load(cookie,"/usr/share/file/misc/magic.mgc"));

  const char *music_path="tmp/Skyreach.mp3";
  assert(0==strcmp(magic_file(cookie,music_path),"audio/mpegapplication/octet-stream"));
  gMusic=Mix_LoadMUS(music_path);assert(gMusic);

  for(Effect *eff=effects;eff->filename;++eff){
    assert(!eff->chunk);
    char path[SIZE]="tmp/";
    strcat(path,eff->filename);
    eff->chunk=Mix_LoadWAV2(path);
    assert(eff->chunk);
  }

  // Stop libmagic
  magic_close(cookie);cookie=NULL;

  static_assert(MIX_MAX_VOLUME==128);
  printf("vol %d\n",Mix_VolumeMusic(-1));
  Mix_VolumeMusic(96);
  printf("vol %d\n",Mix_VolumeMusic(-1));

}

void help(){
  printf(BLUE);
  puts(" <9> - play");
  puts(" <0> - stop");
  puts(" <Q> - quit");
  puts("refer to \"effect.c\" for effects and their corresponding keys");
  printf(RESET);fflush(stdout);
}

static void Mix_PlayChannel2(/*const*/ Mix_Chunk *const chunk){
  const int ch=Mix_PlayChannelTimed(-1, chunk, 0/*1 loop*/, 333/*ms*/);
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

void SDL_PollEvent2(){

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
        goto supercontinue;

    // -- Ignore peaceful events - verbose
    if(e.type!=SDL_KEYDOWN){printf("event %u ingored\n",e.type);continue;}
    assert(e.key.type==SDL_KEYDOWN);

    // https://wiki.libsdl.org/SDL_Scancode
    // https://wiki.libsdl.org/SDL_Keycode
    for(Effect *eff=effects;eff->filename;++eff){
      if(eff->scancode==e.key.keysym.scancode){
        assert(eff->sym==e.key.keysym.sym);
        puts(eff->prompt);
        Mix_PlayChannel2(eff->chunk);
        goto supercontinue;
      }
    }

    switch(e.key.keysym.scancode){
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
    // printf("%ums\n",e.key.timestamp);
    // printf("%u\n",e.key.windowID);
    assert(2==e.key.windowID);
    printf("%u\n",e.key.repeat);

    supercontinue:;

  }

}

void Mix_FreeXXX(){
  Mix_HaltMusic2();
  for(Effect *eff=effects;eff->filename;++eff){
    assert(eff->chunk);Mix_FreeChunk(eff->chunk);eff->chunk=NULL;
  }
  assert(gMusic);Mix_FreeMusic(gMusic);gMusic=NULL;
}
