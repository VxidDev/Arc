#include "../../include/object.h"
#include "../../include/mempool.h"

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

static Number _intCache[512];
static bool _intCacheInit = false;

static void _ensureIntCache() {
  if (_intCacheInit) return;

  for (int i = 0; i < 512; i++) {
    _intCache[i].base.type = OBJ_NUMBER_INT;
    _intCache[i].as.i = i - 256;
    _intCache[i].base.isStatic = true;
  }

  _intCacheInit = true;
}

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

/*
static inline bool isValid(const Object* a, const Object* b) {
  ObjType aType = a->type;
  ObjType bType = b->type;

  return ((aType == OBJ_NUMBER_FLOAT || aType == OBJ_NUMBER_INT) && (b->type == OBJ_NUMBER_FLOAT || b->type == OBJ_NUMBER_INT));
}
*/

Number *initInt(int64_t value) {
  _ensureIntCache();

  if (value >= -256 && value < 256)
    return &_intCache[value + 256];

  Number* number = poolAlloc(numberPool);

  if (!number) return NULL;

  number->as.i = value;
  number->base.type = OBJ_NUMBER_INT;
  number->base.isStatic = false;

  return number;
}

Number *initFloat(double value) {
  Number* number = poolAlloc(numberPool);

  if (!number) return NULL;

  number->as.f = value;
  number->base.type = OBJ_NUMBER_FLOAT;
  number->base.isStatic = false;

  return number;
}

Number *copyNumber(Number *num) {
  if (!num) return NULL;

  Number* n = poolAlloc(numberPool);
  if (!n) return NULL;

  *n = *num;
  n->base.isStatic = false;
  return n;
}

ErrType addNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  // if (!isValid((Object*)dest, (Object*)src)) return ERR_TYPE;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.f = toDouble(src) + toDouble(dest);
    dest->base.type = OBJ_NUMBER_FLOAT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) + toLong(dest);
  return ERR_NONE;
}

ErrType subNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  // if (!isValid((Object*)dest, (Object*)src)) return ERR_TYPE;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.f = toDouble(src) - toDouble(dest);
    dest->base.type = OBJ_NUMBER_FLOAT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) - toLong(dest);
  return ERR_NONE;
}

ErrType divNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  // if (!isValid((Object*)dest, (Object*)src)) return ERR_TYPE;

  double destVal = toDouble(dest);

  if (!destVal) {
    return ERR_DIV_BY_ZERO;
  }

  dest->base.type = OBJ_NUMBER_FLOAT;

  dest->as.f = toDouble(src) / destVal;
  return ERR_NONE;
}

ErrType mulNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  // if (!isValid((Object*)dest, (Object*)src)) return ERR_TYPE;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.f = toDouble(src) * toDouble(dest);
    dest->base.type = OBJ_NUMBER_FLOAT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) * toLong(dest);
  return ERR_NONE;
}

ErrType powNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;
  // if (!isValid((Object*)dest, (Object*)src)) return ERR_TYPE;

  double base = toDouble(src);
  double exp  = toDouble(dest);

  dest->base.type = OBJ_NUMBER_FLOAT;
  dest->as.f = pow(base, exp);

  return ERR_NONE;
}

ErrType isEqualNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.i = toDouble(dest) == toDouble(src);
    dest->base.type = OBJ_NUMBER_INT;
    return ERR_NONE;
  }

  dest->as.i = toLong(dest) == toLong(src);
  return ERR_NONE;
}

ErrType isNotEqualNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.i = toDouble(src) != toDouble(dest);
    dest->base.type = OBJ_NUMBER_INT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) != toLong(dest);
  return ERR_NONE;
}

ErrType isLessThanEqualNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.i = toDouble(src) <= toDouble(dest);
    dest->base.type = OBJ_NUMBER_INT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) <= toLong(dest);
  return ERR_NONE;
}

ErrType isGreaterThanEqualNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.i = toDouble(src) >= toDouble(dest);
    dest->base.type = OBJ_NUMBER_INT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) >= toLong(dest);
  return ERR_NONE;
}

ErrType isLessThanNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;
  
  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.i = toDouble(src) < toDouble(dest);
    dest->base.type = OBJ_NUMBER_INT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) < toLong(dest);
  return ERR_NONE;
}

ErrType isGreaterThanNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.i = toDouble(src) > toDouble(dest);
    dest->base.type = OBJ_NUMBER_INT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) > toLong(dest);
  return ERR_NONE;
}

ErrType andNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.i = toDouble(src) && toDouble(dest);
    dest->base.type = OBJ_NUMBER_INT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) && toLong(dest);
  return ERR_NONE;
}

ErrType orNumber(Number* dest, const Number* src) {
  if (!dest || !src) return ERR_NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    dest->as.i = toDouble(src) || toDouble(dest);
    dest->base.type = OBJ_NUMBER_INT;
    return ERR_NONE;
  }

  dest->as.i = toLong(src) || toLong(dest);
  return ERR_NONE;
}


