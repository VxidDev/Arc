#ifndef VALUE_H
#define VALUE_H

typedef enum {
  VAL_UNDEF, // Undefined value, there to raise NameError on read
  VAL_INT,
  VAL_FLOAT,
  VAL_OBJ,  // heap-allocated: string, list, function, etc
} ValueType;

typedef struct {
  ValueType type;

  union {
    int64_t i;
    double f;
    Object *obj;
  } as;

} Value;

void freeValue(Value v);
Value copyValue(Value v);

// Constructors
#define VAL_INT(n) ((Value){ VAL_INT, { .i = (n) } })
#define VAL_FLOAT(n) ((Value){ VAL_FLOAT, { .f = (n) } })
#define VAL_OBJ(o) ((Value){ VAL_OBJ, { .obj = (o) } })
#define VAL_UNDEF() ((Value){ VAL_UNDEF })

// Checks
#define IS_INT(v) ((v).type == VAL_INT)
#define IS_FLOAT(v) ((v).type == VAL_FLOAT)
#define IS_OBJ(v) ((v).type == VAL_OBJ)
#define IS_UNDEF(v) ((v).type == VAL_UNDEF)
#define AS_INT(v) ((v).as.i)
#define AS_FLOAT(v) ((v).as.f)
#define AS_OBJ(v) ((v).as.obj)

#endif // VALUE_H
