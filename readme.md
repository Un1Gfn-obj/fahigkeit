## [fahigkeit](https://github.com/Un1Gfn-obj/fahigkeit)

https://osu.ppy.sh/beatmapsets/190851

https://osu.ppy.sh/wiki/en/osu!_File_Formats/Osu_(file_format)

    glxinfo | grep -i opengl | grep -i -e vendor -e version
    env LIBGL_ALWAYS_SOFTWARE=1 glxinfo | grep -i opengl | grep -i -e vendor -e version

**soft-hitwhistle.wav**

Mix_Chunk \
[Mix_LoadMUS](https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_55.html) \
[Mix_LoadWAV](https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_19.html) \
Mix_Music \
SDL_Init(SDL_INIT_AUDIO) \
SDL_LoadWAV \
SDL_mixer \
SDL_sound

sdl timer

Audio API too low level, use SDL_mixer or OpenAL instead

SDL timer ms ticks

SDL reference in [Wiki](https://wiki.libsdl.org/SDL_Init) form

SDL_mixer reference in [HTML](https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_frame.html) form

zip.c ->
buf ->
[SDL_RWFromMem()](https://wiki.libsdl.org/SDL_RWFromMem) ->
[SDL_RWops](https://wiki.libsdl.org/SDL_RWops) ->
Mix_LoadWAV_RW()

https://cpp.hotexamples.com/examples/-/-/Mix_QuickLoad_WAV/cpp-mix_quickload_wav-function-examples.html \
http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c \
https://gist.github.com/phenyque/0137d70cad0ccababfd9b7629ae1b5aa \
zip.c ->
buf ->
extract RAW from WAV for Mix_QuickLoad_WAV(), and metadata for playing
