// pkgs="sdl2,glew" && gcc -std=gnu11 -Wall -Wextra -Werror=shadow $(pkg-config --cflags "$pkgs") -o gl.out gl.c $(pkg-config --libs "$pkgs") && ./gl.out

// pkgs="sdl2,glew" && cscope $(pkg-config --cflags-only-I "$pkgs") -1 GLfloat gl.c

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "./def.h"

#define MY_MAJOR 4
#define MY_MINOR 6
#define MY_CONTEXT_PROFILE_MASK SDL_GL_CONTEXT_PROFILE_CORE
// #define MY_CONTEXT_RELEASE SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH // Set succeed with no effect

// Not working
// #define MY_ACCELERATED_VISUAL 0 // force software rendering // env LIBGL_ALWAYS_SOFTWARE=1 ./fahigkeit.out
#define MY_ACCELERATED_VISUAL 1 // require hardware acceleration

#define WIDTH 320
#define HEIGHT 240

static SDL_Window *gWindow=NULL;
static SDL_GLContext *gContext=NULL;
static unsigned int shaderProgram=0;

static unsigned int VAO=GL_INVALID_VALUE; // Vertex Array Object
static unsigned int VBO=GL_INVALID_VALUE; // Vertex Buffer Object

static void SDL_GL_SetAttribute2(){

  assert(0==SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,MY_ACCELERATED_VISUAL));

  // SDL_GL_CONTEXT_PROFILE_MASK determines the type of context created
  // SDL_GL_CONTEXT_MAJOR_VERSION and SDL_GL_CONTEXT_MINOR_VERSION determine which version
  // Must be set prior to creating the first window
  // Can't change SDL_GL_CONTEXT_PROFILE_MASK without first destroying all windows created with the previous setting.
  assert(0==SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,MY_MAJOR));
  assert(0==SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,MY_MINOR));
  assert(0==SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,MY_CONTEXT_PROFILE_MASK));

}

static void SDL_GL_GetAttribute2(){

  int v=-1;

  printf("min: ");
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_RED_SIZE,&v));printf(RED "%d-bit " RESET,v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE,&v));printf(GREEN "%d-bit " RESET,v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE,&v));printf(BLUE "%d-bit " RESET,v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE,&v));assert(0==v); // printf("alphaCH %d-bit ",v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE,&v));printf("frameBuf %d-bit ",v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE,&v));printf("depthBuf %d-bit ",v);
  puts("");

  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER,&v));printf("doubleBuf: %s\n",v?"ON":"OFF");

  // DL_GL_GetAttribute(SDL_GL_STENCIL_SIZE,&v) // Error 502

  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS,&v));assert(0==v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES,&v));assert(0==v);
  puts("MSAA: OFF");

  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL,&v));assert(MY_ACCELERATED_VISUAL==v);
  printf("ACCEL: %s\n",v?"REQUIRE_HW":"FORCE_SW");

  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,&v));assert(MY_MAJOR==v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,&v));assert(MY_MINOR==v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS,&v));assert(0==v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,&v));assert(MY_CONTEXT_PROFILE_MASK==v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT,&v));assert(0==v);

}

static void glGetString2(){
  const char *s=NULL;
  s=(const char*)glGetString(GL_VENDOR);assert(s);puts(s);
  assert(0!=strcmp(s,"Mesa/X.org"));
  assert(0==strcmp(s,"Intel"));
  s=(const char*)glGetString(GL_RENDERER);assert(s);puts(s);
  s=(const char*)glGetString(GL_VERSION);assert(s);puts(s);
  s=(const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);assert(s);puts(s);
}

static unsigned int glCompileShader2(const char *const path,const GLenum shaderType){

  FILE *file=fopen(path,"r");assert(file);
  assert(
    0==fseek(file,0,SEEK_END) &&
    0==feof(file) &&
    0==ferror(file));
  const long l=ftell(file); // Excluding null character
  rewind(file);
  assert(1==sizeof(GLchar));
  GLchar buf[l+1];
  bzero(buf,l+1);
  assert((long long)l==(long long)fread(buf,1,l,file));
  assert(buf[l]=='\0');
  assert(buf[l-1]!='\0');

  int success=0;
  char infoLog[SIZE]={};
  unsigned int shader=glCreateShader(shaderType);
  // const GLchar *string=buf;
  // glShaderSource(shader,1,&string,NULL);
  // assert(buf==string);
  glShaderSource(shader,1,&(const GLchar*){buf},NULL);
  glCompileShader(shader);
  glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
  if(0==success){
    glGetShaderInfoLog(shader, SIZE, NULL, infoLog);
    assert(false);
  }
  return shader;

}

static void glLinkProgram2(){
  GLuint vertexShader=glCompileShader2("vertex.glsl",GL_VERTEX_SHADER);
  GLuint fragmentShader=glCompileShader2("fragment.glsl",GL_FRAGMENT_SHADER);
  int success=0;
  char infoLog[SIZE]={};
  // Link shader program
  assert(1<=(shaderProgram=glCreateProgram()));
  glAttachShader(shaderProgram,vertexShader);
  glAttachShader(shaderProgram,fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if(0==success){bzero(infoLog,SIZE);glGetShaderInfoLog(shaderProgram, SIZE, NULL, infoLog);assert(false);}
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

static void drawTriangle(const float *const vertices){

  glGenVertexArrays(1, &VAO);assert(GL_INVALID_VALUE!=VAO);glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);assert(GL_INVALID_VALUE!=VBO);glBindBuffer(GL_ARRAY_BUFFER,VBO);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe polygons.

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(shaderProgram);
  glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0); // no need to unbind it every time 

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  SDL_GL_SwapWindow(gWindow);

}

int main(){

  puts("");
  SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_VIDEO);
  SDL_GL_SetAttribute2();
  assert((gWindow=SDL_CreateWindow(
    "SDL Tutorial",
    // SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
    1,1,
    WIDTH,HEIGHT,
    SDL_WINDOW_OPENGL|SDL_WINDOW_BORDERLESS)));
  assert((gContext=SDL_GL_CreateContext(gWindow)));
  assert(GLEW_OK==glewInit());
  assert(0==SDL_GL_SetSwapInterval(-1));
  SDL_GL_GetAttribute2();

  puts("");
  glGetString2();

  glViewport(0, 0, WIDTH, HEIGHT);

  glLinkProgram2();

  drawTriangle((const float[]){
    -0.5f, -0.5f, 0.0f, // left  
     0.5f, -0.0f, 0.0f, // right 
     0.0f,  0.5f, 0.0f  // top   
  });
  getchar();

  drawTriangle((const float[]){
    -0.5f, -0.4f, 0.0f, // left
     0.5f, -0.5f, 0.0f, // right
     0.0f,  0.5f, 0.0f  // top
  });
  getchar();

  glDeleteProgram(shaderProgram);

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(gWindow);
  getchar();

  SDL_GL_DeleteContext(gContext);gContext=NULL;
  assert(gWindow);SDL_DestroyWindow(gWindow);gWindow=NULL;
  SDL_Quit();

  return 0;

}
