/*

pkgs="sdl2,glew" && cscope $(pkg-config --cflags-only-I "$pkgs") -1 y1 gl.c

pkgs="sdl2,glew" \
printf "%s\n%s\n" "$(xxd -i vertex.glsl)" | sed -e "s/unsigned char/static const GLchar/g" -e "s/unsigned int/static const GLint/g" >glsl.h \
&& gcc -std=gnu11 -g -Wall -Wextra -Werror=shadow $(pkg-config --cflags "$pkgs") -o gl.out -xc gl.c $(pkg-config --libs "$pkgs") \
&& ./gl.out

https://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967

*/

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "./def.h"
#include "./monokai.h"

// NOT null-terminated
#include "./glsl.h"

#define MY_MAJOR 4
#define MY_MINOR 6
#define MY_CONTEXT_PROFILE_MASK SDL_GL_CONTEXT_PROFILE_CORE
// #define MY_CONTEXT_RELEASE SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH // Set succeed with no effect

// MSAA
#define MY_MULTISAMPLEBUFFERS 1
#define MY_MULTISAMPLESAMPLES 16

// Not working
// #define MY_ACCELERATED_VISUAL 0 // force software rendering // env LIBGL_ALWAYS_SOFTWARE=1 ./fahigkeit.out
#define MY_ACCELERATED_VISUAL 1 // require hardware acceleration

// #define WIDTH 320
// #define HEIGHT 240
#define WIDTH 640
// #define HEIGHT 480
#define HEIGHT WIDTH

static SDL_Window *gWindow=NULL;
static SDL_GLContext *gContext=NULL;

static GLuint VAO=GL_INVALID_VALUE; // Vertex Array Object
static GLuint VBO=GL_INVALID_VALUE; // Vertex Buffer Object

const char *const fragmentShaderSource0=
"#version 330 core\n\
out vec4 FragColor;\n\
void main(){FragColor=vec4(";
const char *const fragmentShaderSource2=
");}\n";

static void SDL_GL_SetAttribute2(){

  assert(0==SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,MY_MULTISAMPLEBUFFERS));
  assert(0==SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,MY_MULTISAMPLESAMPLES));
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

  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS,&v));assert(MY_MULTISAMPLEBUFFERS==v);
  v=-1;assert(0==SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES,&v));assert(MY_MULTISAMPLESAMPLES==v);
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

#define checkCompile(s,S) glGetXXXInfoLog2(s,S,glGetShaderiv,GL_COMPILE_STATUS,glGetShaderInfoLog)
#define checkLink(s,P) glGetXXXInfoLog2(s,P,glGetProgramiv,GL_LINK_STATUS,glGetProgramInfoLog)
static void glGetXXXInfoLog2(const char *const hint,GLuint id,void(*glGetXXXiv)(GLuint,GLenum,GLint*),GLenum pname,void (*glGetXXXInfoLog)(GLuint,GLsizei,GLsizei*,GLchar*)){
  GLint success=0;
  (*glGetXXXiv)(id,pname,&success);
  if(GL_TRUE==success)
    return;
  // GLint logSize=SIZE;
  GLint logSize=-1;
  (*glGetXXXiv)(id,GL_INFO_LOG_LENGTH,&logSize);assert(logSize>=1);
  GLchar infoLog[logSize];
  bzero(infoLog,logSize);
  (*glGetXXXInfoLog)(id,logSize,NULL,infoLog);
  puts("");
  printf("%s:\n",hint);
  printf(RED);
  puts(infoLog);
  printf(RESET);fflush(stdout);
  assert(false);
}

static GLuint glLinkProgram2(const GLchar *const color){

  GLuint vertexShader=glCreateShader(GL_VERTEX_SHADER);
  assert(vertexShader>=1);
  glShaderSource(vertexShader,1,&(const GLchar*){vertex_glsl},&vertex_glsl_len);
  glCompileShader(vertexShader);checkCompile("vertexShader",vertexShader);

  GLuint fragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
  assert(fragmentShader>=1);
  // puts("---");
  // puts(fragmentShaderSource0);
  // puts("---");
  // puts(HEX2GLSL(MK_BLACK));
  // puts("---");
  // puts(fragmentShaderSource2);
  // puts("---");
  glShaderSource(fragmentShader,3,(const char *const [3]){
    fragmentShaderSource0,
    color,
    fragmentShaderSource2},
  NULL);
  glCompileShader(fragmentShader);checkCompile("fragmentShader",fragmentShader);

  GLuint program=glCreateProgram();
  assert(1<=program);
  glAttachShader(program,vertexShader);
  glAttachShader(program,fragmentShader);
  glLinkProgram(program);checkLink("link",program);

  glDeleteShader(vertexShader);vertexShader=0;
  glDeleteShader(fragmentShader);fragmentShader=0;

  return program;

}

static void vxo(){

  glGenBuffers(1, &VBO);assert(GL_INVALID_VALUE!=VBO);
  glBindBuffer(GL_ARRAY_BUFFER,VBO);

  glGenVertexArrays(1, &VAO);assert(GL_INVALID_VALUE!=VAO);
  glBindVertexArray(VAO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), NULL);

  // We can glBindBuffer() now since glVertexAttribPointer() has registered VBO as the vertex attribute's bound vertex buffer object
  // However, to draw a new triangle, coordinates in the buffer should be modifed by glBufferData()
  // Therefore we leave the buffer intact
  // glBindBuffer(GL_ARRAY_BUFFER,0);

}

static void vxoClear(){
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
}

typedef GLfloat const Triangle[9];

// Pokayoke
#define PT(X,Y) X,Y
#define drawTriangle(PT0,PT1,PT2,SCALE) drawTriangle0(PT0,PT1,PT2,SCALE)
#define drawTriangle0(X0,Y0,X1,Y1,X2,Y2,SCALE) drawTriangle00((const GLfloat[9]){\
  (GLfloat)(SCALE)*(GLfloat)(X0),\
  (GLfloat)(SCALE)*(GLfloat)(Y0),\
  0.0f,\
  (GLfloat)(SCALE)*(GLfloat)(X1),\
  (GLfloat)(SCALE)*(GLfloat)(Y1),\
  0.0f,\
  (GLfloat)(SCALE)*(GLfloat)(X2),\
  (GLfloat)(SCALE)*(GLfloat)(Y2),\
  0.0f})
static void drawTriangle00(const GLfloat *const vertices){
  // assert((GLfloat)(y1)==-0.0f);
  // printf("%f\n",y1);
  glBufferData(GL_ARRAY_BUFFER,9*sizeof(GLfloat),vertices,GL_STATIC_DRAW);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe polygons.
  glDrawArrays(GL_TRIANGLES,0,3);
  SDL_GL_SwapWindow(gWindow);
}

int main(){

  // puts("");
  // puts(HEX2GLSL(MK_GREEN));

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
  // SDL_GL_GetSwapInterval()
  SDL_GL_GetAttribute2();

  puts("");
  glGetString2();

  glViewport(0, 0, WIDTH, HEIGHT);

  GLuint redProgram=glLinkProgram2(HEX2GLSL(MK_RED));
  GLuint greenProgram=glLinkProgram2(HEX2GLSL(MK_GREEN));
  GLuint blueProgram=glLinkProgram2(HEX2GLSL(MK_BLUE));

  vxo();

  glClearColor(HEX2RGBA(MK_WHITE));glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(gWindow);
  getchar();
  // SDL_GL_SwapWindow(gWindow);
  // getchar();

  glClearColor(HEX2RGBA(MK_BLACK));glClear(GL_COLOR_BUFFER_BIT);
  static_assert(sizeof(GLfloat)==sizeof(float));

  glUseProgram(redProgram);drawTriangle(
    PT(-1.0f,+sqrtf(3.0f)),
    PT(+1.0f,+sqrtf(3.0f)),
    PT(+0.0f,+0.0f),
    0.5f);
  glUseProgram(greenProgram);drawTriangle(
    PT(+1.0f,-sqrtf(3.0f)),
    PT(+2.0f,+0.0f),
    PT(+0.0f,+0.0f),
    0.5f);
  glUseProgram(blueProgram);drawTriangle(
    PT(-1.0f,-sqrtf(3.0f)),
    PT(-2.0f,+0.0f),
    PT(+0.0f,+0.0f),
    0.5f);
  getchar();

  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(redProgram);drawTriangle(
    PT(+sqrtf(3.0f),+1.0f),
    PT(-sqrtf(3.0f),+1.0f),
    PT(+0.0f,+0.0f),
    0.4f);
  glUseProgram(greenProgram);drawTriangle(
    PT(+sqrtf(3.0f),+1.0f),
    PT(+0.0f,-2.0f),
    PT(+0.0f,+0.0f),
    0.4f);
  glUseProgram(blueProgram);drawTriangle(
    PT(-sqrtf(3.0f),+1.0f),
    PT(+0.0f,-2.0f),
    PT(+0.0f,+0.0f),
    0.4f);
  getchar();

  vxoClear();

  glClearColor(HEX2RGBA(MK_GRAY));
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(gWindow);
  getchar();

  glDeleteProgram(redProgram);redProgram=0;
  glDeleteProgram(greenProgram);greenProgram=0;
  glDeleteProgram(blueProgram);blueProgram=0;

  SDL_GL_DeleteContext(gContext);gContext=NULL;
  assert(gWindow);SDL_DestroyWindow(gWindow);gWindow=NULL;
  SDL_Quit();

  return 0;

}
