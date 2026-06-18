#ifndef IO_H 
#define IO_H 

#include "../object.h"

Object* builtIn_print(Object** args, size_t argCount);
Object* builtIn_get_input(Object** args, size_t argCount);

// File I/O 
Object* builtIn_open_file(Object** args, size_t argCount);
Object* builtIn_close_file(Object** args, size_t argCount);
Object* builtIn_read_file(Object** args, size_t argCount);
Object* builtIn_write_file(Object** args, size_t argCount);

Object* builtIn_stream_read_char(Object** args, size_t argCount);
Object* builtIn_stream_seek(Object** args, size_t argCount);
Object* builtIn_stream_tell(Object** args, size_t argCount);

#endif // !IO_H 
