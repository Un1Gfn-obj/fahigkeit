## [fahigkeit](https://github.com/Un1Gfn-obj/fahigkeit)

https://osu.ppy.sh/beatmapsets/190851

https://osu.ppy.sh/wiki/en/osu!_File_Formats/Osu_(file_format)

    glxinfo | grep -i opengl | grep -i -e vendor -e version
    env LIBGL_ALWAYS_SOFTWARE=1 glxinfo | grep -i opengl | grep -i -e vendor -e version

**soft-hitwhistle.wav**

Mix_Music \
SDL_sound

sdl timer

Audio API too low level, use SDL_mixer or OpenAL instead

SDL timer ms ticks

SDL reference in [Wiki](https://wiki.libsdl.org/SDL_Init) form

SDL_mixer reference in [HTML](https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_frame.html) form

http://docs.gl/

Draw everything with OpenGL, then show this image with a platform-dependent buffer swapping command


Screen Tearing - monitor not ready for the next frame from GPU
Screen Stuttering - monitor ready to receive the next frame but GPU hasn't sent it
Screen Flickering - intermediate image where the entire page is cleared but next page is not yet drawn

Double Buffering with software \
front buffer in VRAM \
back buffer in RAM

Double Buffering with Page Flipping \
both buffers in VRAM
