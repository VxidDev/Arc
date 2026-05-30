#include "../../include/object.h"
#include "../../include/utils.h"

#include <stdlib.h>

File* copyFile(File* f) {
  return initFile(f->file, f->fname, f->fmod);
}

File* initFile(FILE* file, char* fname, char *fmod) {
  if (!file) return NULL;

  File* fileObj = malloc(sizeof(File));

  if (!fileObj) return NULL;
  
  fileObj->base.type = OBJ_FILE;

  fileObj->file = file;
  fileObj->fname = stringDup(fname);
  fileObj->fmod = stringDup(fmod);

  return fileObj;
}
