#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h> // basename

// https://github.com/nih-at/libzip
// #include <zip.h>
// https://github.com/madler/zlib/tree/master/contrib/minizip
#include <minizip/unzip.h>

#include "./effect.h"
#include "./def.h"

static void fwrite2(const void *restrict ptr, size_t size, size_t nmemb,const char *const path){
  FILE *stream=fopen(path,"wb");
  assert(stream);
  assert(size*nmemb==fwrite(ptr,size,nmemb,stream));
  assert(0==fclose(stream));
  stream=NULL;
}

static void UNTITLED(const unzFile file){

  unz_file_info file_info={};
  char szFileName[SIZE]={};
  assert(UNZ_OK==unzGetCurrentFileInfo(
    file,       &file_info,
    szFileName, SIZE,
    NULL,       0,
    NULL,       0
  ));
  assert(file_info.size_filename==strlen(szFileName));
  assert(file_info.size_file_extra==0UL);
  assert(file_info.size_file_comment==0UL);

  const char *s=strrchr(szFileName,'.');
  assert(s); // Suffix is required

  bool skip=true;

  if(0==strcmp(s,".mp3"))
    skip=false;

  if(0==strcmp(s,".wav")){
    for(Effect *eff=effects;eff->filename;++eff){
      if(0==strcmp(szFileName,eff->filename)){
        skip=false;
        break;
      }
    }
  }

  if(!skip){

    // puts(szFileName);
    assert(UNZ_OK==unzOpenCurrentFile(file));
    #define x (file_info.uncompressed_size)
    #define y (file_info.uncompressed_size*2)
    // printf("expect %lu bytes\n",x);
    assert(x>=1);
    unsigned char buf[y];
    bzero(buf,y);
    printf("extracting %s ...\n",szFileName);
    assert(x==(unsigned long)unzReadCurrentFile(file,buf,y));
    // printf("read %d bytes\n",unzReadCurrentFile(file,buf,y));
    static_assert(sizeof(unsigned char)==1);
    char path[SIZE]="tmp/";
    strcat(path,szFileName);
    fwrite2(buf,1,x,path);
    #undef x
    #undef y
    assert(UNZ_OK==unzCloseCurrentFile(file));

  }

}

void unzip(){

  unzFile file=unzOpen("/home/darren/fahigkeit/190851 Amamiya Sora - Skyreach [no video].osz");
  assert(file);

  unz_global_info global_info={};
  assert(UNZ_OK==unzGetGlobalInfo(file,&global_info));
  printf("%lu files\n",global_info.number_entry);
  assert(0==global_info.size_comment);
  char szComment[SIZE]={};
  assert(0==unzGetGlobalComment(file,szComment,SIZE));

  assert(UNZ_OK==unzGoToFirstFile(file));
  UNTITLED(file);
  for(;;){
    const int r=unzGoToNextFile(file);
    if(r==UNZ_END_OF_LIST_OF_FILE)
      break;
    assert(r==UNZ_OK);
    UNTITLED(file);
  }

  assert(UNZ_OK==unzClose(file));
  file=NULL;

}