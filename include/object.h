#ifndef OBJECT_H
#define OBJECT_H

typedef enum ObjType {
  OBJ_NUMBER
} ObjType;

typedef struct Object {
  ObjType type;
} Object;

typedef struct Number {
  Object base;
  long value;
} Number;

Number* initNumber(long value);

Number* addNumber(const Number* dest, const Number* src);
Number* subNumber(const Number* dest, const Number* src);
Number* mulNumber(const Number* dest, const Number* src);
Number* divNumber(const Number* dest, const Number* src);

#endif // OBJECT_H
