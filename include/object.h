#ifndef OBJECT_H
#define OBJECT_H

typedef enum ObjType {
  OBJ_NUMBER_INT,
  OBJ_NUMBER_FLOAT,
} ObjType;

typedef struct Object {
  ObjType type;
} Object;

typedef struct Number {
  Object base;

  union {
    long i;
    double f;
  } as;

} Number;

Number* initInt(long value);
Number* initFloat(double value);

Number* addNumber(const Number* dest, const Number* src);
Number* subNumber(const Number* dest, const Number* src);
Number* mulNumber(const Number* dest, const Number* src);
Number* divNumber(const Number* dest, const Number* src);

#endif // OBJECT_H
