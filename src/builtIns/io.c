#include "../../include/builtIns/io.h"

#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

static void printInternal(Object* obj) {
  switch (obj->type) {
    case OBJ_NUMBER_INT:
      printf("%" PRId64, ((Number*)obj)->as.i);
      break;

    case OBJ_NUMBER_FLOAT:
      printf("%f", ((Number*)obj)->as.f);
      break;

    case OBJ_STRING:
      printf("%s", ((String*)obj)->value);
      break;

    case OBJ_LIST: {
      List* list = (List*)obj;
      printf("[");
      for (uint64_t i = 0; i < list->size; i++) {
        printInternal(list->objects[i]);
        if (i + 1 < list->size) printf(", ");
      }
      printf("]");
      break;
    }

    case OBJ_NULL: {
      printf("null");
      break;
    }

    default:
      printf("<object>");
      break;
  }
}

Object* builtIn_print(Object** args, size_t argCount) {
  for (size_t i = 0; i < argCount; i++) {
    printInternal(args[i]);
    if (i + 1 < argCount) printf(" ");
  }

  printf("\n");
  fflush(stdout);

  return (Object*)initInt(1);
}

Object* builtIn_get_input(Object** args, size_t argCount) {
  if (argCount > 1) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected at most 1 argument, received %zu.", argCount);

    return (Object*)initProgramError(buf);
  }

  Object* arg = args[0];

  if (arg->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument of type 'string', received '%s'.", typeofobj(arg));

    return (Object*)initProgramError(buf);
  }

  printf("%s", ((String*)arg)->value);
  
  size_t size = 0;
  size_t capacity = 256;

  char* buf = malloc(capacity);

  if (!buf) {
    return (Object*)initProgramError("Unable to read input from user.");
  }

  int c = 0;

  while ((c = getchar()) != EOF && c != '\n') {
    if (size >= capacity) {
      capacity *= 2;

      void* tmp = realloc(buf, capacity);

      if (!tmp) {
        free(buf);
        return (Object*)initProgramError("Unable to read input from user.");
      }

      buf = tmp;
    }

    buf[size++] = c;
  }

  buf[size] = '\0';

  Object* s = (Object*)noCopyInitString(buf, size);
  return s;
}

Object* builtIn_open_file(Object** args, size_t argCount) {
  (void)argCount;

  Object* fnameObj = args[0];
  Object* fmodeObj = args[1];

  if (fnameObj->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected object of type 'string' for file name, received object of type '%s'.", typeofobj(fnameObj));

    return (Object*)initProgramError(buf);
  }

  if (fmodeObj->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected object of type 'string' for file mode, received object of type '%s'.", typeofobj(fmodeObj));

    return (Object*)initProgramError(buf);
  }

  String* fname = (String*)fnameObj;
  String* fmode = (String*)fmodeObj;

  FILE* file = fopen(fname->value, fmode->value);

  if (!file) {
    char buf[1024];

    snprintf(buf, sizeof(buf), "Failed to open file: '%s'.", fname->value);

    return (Object*)initProgramError(buf);
  }

  return (Object*)initFile(file, fname->value, fmode->value);
}

Object* builtIn_close_file(Object** args, size_t argCount) {
  (void)argCount;

  Object* arg = args[0];

  if (arg->type != OBJ_FILE) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument of type 'file', received '%s'.", typeofobj(arg));

    return (Object*)initProgramError(buf);
  }
  
  File* file = (File*)arg;

  if (file->file) {
    fclose(file->file);
    file->file = NULL;
  }

  return (Object*)initInt(1);
}

Object* builtIn_read_file(Object** args, size_t argCount) {
  (void)argCount;

  Object* arg = args[0];

  if (arg->type != OBJ_FILE) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument of type 'file', received '%s'.", typeofobj(arg));

    return (Object*)initProgramError(buf);
  }

  File* file = (File*)arg;

  if (!file->file) {
    return (Object*)initProgramError("File stream is closed.");
  }

  if (fseek(file->file, 0, SEEK_END) != 0) {
    return (Object*)initProgramError("Failed to get file's size.");
  }

  long size_long = ftell(file->file);

  if (fseek(file->file, 0, SEEK_SET) != 0) {
    return (Object*)initProgramError("Failed to reset file's position.");
  }

  if (size_long < 0) {
    return (Object*)initProgramError("Failed to get file's size.");
  }

  size_t size = (size_t)size_long;

  char* fcontent = malloc(size + 1);

  if (!fcontent) {
    return (Object*)initProgramError("Failed to read file's content.");
  }

  size_t read = fread(fcontent, 1, size, file->file);

  if (read != size) {
    free(fcontent);
    
    if (ferror(file->file)) {
      return (Object*)initProgramError("Failed to read file's content (I/O error).");
    }

    if (feof(file->file)) {
      return (Object*)initProgramError("Encountered EOF before reading full file.");
    }

    return (Object*)initProgramError("Failed to read file's content.");
  }

  fcontent[size] = '\0';

  Object* obj = (Object*)noCopyInitString(fcontent, size);
  return obj;
}

Object* builtIn_write_file(Object** args, size_t argCount) {
  (void)argCount;
  Object* arg0 = args[0];

  if (arg0->type != OBJ_FILE) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'file', received '%s'.", typeofobj(arg0));

    return (Object*)initProgramError(buf);
  }

  Object* arg1 = args[1];

  if (arg1->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument 2 to be object of type 'string', received '%s'.", typeofobj(arg1));

    return (Object*)initProgramError(buf);
  }

  File* file = (File*)arg0;
  String* string = (String*)arg1;

  if (!file->file) {
    return (Object*)initProgramError("File is already closed.");
  }

  size_t written = fwrite(string->value, 1, string->len, file->file);

  if (written != string->len) {
    if (ferror(file->file)) {
      return (Object*)initProgramError("Could not write content to file.");
    }
  }

  return (Object*)initInt(1);
}

Object* builtIn_stream_read_char(Object** args, size_t argCount) {
  (void)argCount;
  
  Object* src = args[0];

  if (src->type != OBJ_FILE) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'file', received '%s'.", typeofobj(src));
    return (Object*)initProgramError(buf);
  }

  File* f = (File*)src;

  if (!f->file) {
    return (Object*)initProgramError("File stream is closed.");
  }

  int c = getc(f->file);

  if (c == EOF) {
    return (Object*)initInt(-1);
  }

  char s[] = {c, 0};

  return (Object*)initString(s, 1);
}

Object* builtIn_stream_seek(Object** args, size_t argCount) {
  (void)argCount;

  Object* stream = args[0];
  Object* offset = args[1];
  Object* whence = args[2];

  if (stream->type != OBJ_FILE) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'file', received '%s'.", typeofobj(stream));
    return (Object*)initProgramError(buf);
  }

  if (offset->type != OBJ_NUMBER_INT) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 2 to be object of type 'int', received '%s'.", typeofobj(offset));
    return (Object*)initProgramError(buf);
  }

  if (whence->type != OBJ_NUMBER_INT) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 3 to be object of type 'int', received '%s'.", typeofobj(whence));
    return (Object*)initProgramError(buf);
  }

  File* f = (File*)stream;

  if (!f->file) {
    return (Object*)initProgramError("File stream is closed.");
  }

  Number* w = (Number*)whence;
  Number* o = (Number*)offset;

  if (w->as.i != SEEK_SET && w->as.i != SEEK_CUR && w->as.i != SEEK_END) {
    return (Object*)initProgramError("Invalid whence.");
  }

  if (o->as.i < 0 && w->as.i == SEEK_SET) {
    return (Object*)initProgramError("Negative offset is not allowed with SEEK_SET.");
  }

  if (fseek(f->file, o->as.i, w->as.i) != 0) {
    char buf[512];
    snprintf(buf, sizeof(buf), "fseek error: %s", strerror(errno));
    return (Object*)initProgramError(buf);
  }
  
  return (Object*)initInt(1);
}

Object* builtIn_stream_tell(Object** args, size_t argCount) {
  (void)argCount;

  Object* stream = args[0];

  if (stream->type != OBJ_FILE) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'file', received '%s'.", typeofobj(stream));
    return (Object*)initProgramError(buf);
  }

  File* f = (File*)stream;

  if (!f->file) {
    return (Object*)initProgramError("File stream is closed.");
  }
  
  long pos = ftell(f->file);

  if (pos == -1L) {
    char buf[512];
    snprintf(buf, sizeof(buf), "ftell error: %s", strerror(errno));
    return (Object*)initProgramError(buf);
  }

  return (Object*)initInt(pos);
}
