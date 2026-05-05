#include "../../include/object.h"

#include <stdlib.h>
#include <math.h>

static inline double toDouble(const Number* n) {
  return (n->base.type == OBJ_NUMBER_FLOAT)
    ? n->as.f
    : (double)n->as.i;
}

static inline long toLong(const Number* n) {
  return (n->base.type == OBJ_NUMBER_INT)
    ? n->as.i
    : (long)n->as.f;
}

static inline ObjType promote(const Number* a, const Number* b) {
  return (a->base.type == OBJ_NUMBER_FLOAT ||
          b->base.type == OBJ_NUMBER_FLOAT)
    ? OBJ_NUMBER_FLOAT
    : OBJ_NUMBER_INT;
}

static inline bool isValid(const Object* a, const Object* b) {
  return ((a->type == OBJ_NUMBER_FLOAT || a->type == OBJ_NUMBER_INT) && (b->type == OBJ_NUMBER_FLOAT || b->type == OBJ_NUMBER_INT));
}

Number *initInt(long value) {
  Number* number = malloc(sizeof(Number));

  if (!number) return NULL;

  number->as.i = value;
  number->base.type = OBJ_NUMBER_INT;

  return number;
}

Number *initFloat(double value) {
  Number* number = malloc(sizeof(Number));

  if (!number) return NULL;

  number->as.f = value;
  number->base.type = OBJ_NUMBER_FLOAT;

  return number;
}

Number *copyNumber(Number *num) {
  if (!num) return NULL;

  if (num->base.type == OBJ_NUMBER_INT) return initInt(num->as.i);
  
  return initFloat(num->as.f);
}

EvalResultNumber addNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};

  if (!isValid((Object*)dest, (Object*)src)) return (EvalResultNumber){NULL, ERR_TYPE};

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    double result = toDouble(src) + toDouble(dest);
    return (EvalResultNumber){initFloat(result), ERR_NONE};
  }

  long result = toLong(src) + toLong(dest);
  return (EvalResultNumber){initInt(result), ERR_NONE};
}

EvalResultNumber subNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  if (!isValid((Object*)dest, (Object*)src)) return (EvalResultNumber){NULL, ERR_TYPE};

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    double result = toDouble(src) - toDouble(dest);
    return (EvalResultNumber){initFloat(result), ERR_NONE};
  }

  long result = toLong(src) - toLong(dest);
  return (EvalResultNumber){initInt(result), ERR_NONE};
}

EvalResultNumber divNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};

  if (!isValid((Object*)dest, (Object*)src)) return (EvalResultNumber){NULL, ERR_TYPE};

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    double destVal = toDouble(dest);

    if (!destVal) {
      return (EvalResultNumber){NULL, ERR_DIV_BY_ZERO};
    }

    double result = toDouble(src) / destVal;
    return (EvalResultNumber){initFloat(result), ERR_NONE};
  }
  
  long destVal = toLong(dest);

  if (!destVal) {
    return (EvalResultNumber){NULL, ERR_DIV_BY_ZERO};
  }

  long result = toLong(src) / destVal;
  return (EvalResultNumber){initInt(result), ERR_NONE};
}

EvalResultNumber mulNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  if (!isValid((Object*)dest, (Object*)src)) return (EvalResultNumber){NULL, ERR_TYPE};

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    double result = toDouble(src) * toDouble(dest);
    return (EvalResultNumber){initFloat(result), ERR_NONE};
  }

  long result = toLong(src) * toLong(dest);
  return (EvalResultNumber){initInt(result), ERR_NONE};
}

EvalResultNumber powNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  if (!isValid((Object*)dest, (Object*)src)) return (EvalResultNumber){NULL, ERR_TYPE}; 

  double result = pow(toDouble(src), toDouble(dest));
  return (EvalResultNumber){initFloat(result), ERR_NONE};
}

EvalResultNumber isEqualNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    return (EvalResultNumber){initInt(toDouble(dest) == toDouble(src)), ERR_NONE};
  }

  return (EvalResultNumber){initInt(toLong(dest) == toLong(src)), ERR_NONE};
}

EvalResultNumber isNotEqualNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    return (EvalResultNumber){initInt(toDouble(src) != toDouble(dest)), ERR_NONE};
  }

  return (EvalResultNumber){initInt(toLong(src) != toLong(dest)), ERR_NONE};
}

EvalResultNumber isLessThanEqualNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    return (EvalResultNumber){initInt(toDouble(src) <= toDouble(dest)), ERR_NONE};
  }

  return (EvalResultNumber){initInt(toLong(src) <= toLong(dest)), ERR_NONE};
}

EvalResultNumber isGreaterThanEqualNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    return (EvalResultNumber){initInt(toDouble(src) >= toDouble(dest)), ERR_NONE};
  }

  return (EvalResultNumber){initInt(toLong(src) >= toLong(dest)), ERR_NONE};
}

EvalResultNumber isLessThanNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    return (EvalResultNumber){initInt(toDouble(src) < toDouble(dest)), ERR_NONE};
  }

  return (EvalResultNumber){initInt(toLong(src) < toLong(dest)), ERR_NONE};
}

EvalResultNumber isGreaterThanNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    return (EvalResultNumber){initInt(toDouble(src) > toDouble(dest)), ERR_NONE};
  }

  return (EvalResultNumber){initInt(toLong(src) > toLong(dest)), ERR_NONE};
}

EvalResultNumber andNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    return (EvalResultNumber){initInt(toDouble(src) && toDouble(dest)), ERR_NONE};
  }

  return (EvalResultNumber){initInt(toLong(src) && toLong(dest)), ERR_NONE};
}

EvalResultNumber orNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return (EvalResultNumber){NULL, ERR_NULL};
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    return (EvalResultNumber){initInt(toDouble(src) || toDouble(dest)), ERR_NONE};
  }

  return (EvalResultNumber){initInt(toLong(src) || toLong(dest)), ERR_NONE};
}


