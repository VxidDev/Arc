IMPORT "__mathlib"

VAR PI = 3.141592653589793
VAR E = 2.718281828459045
VAR TAU = 6.283185307179586

FN exp(x) THEN 
  E ^ x
END 

FN logn(x) THEN
  IF x <= 0 THEN
    RETURN 0
  END 

  VAR y = 0.0
  VAR i = 0

  WHILE i < 20 THEN
    VAR ey = exp(y)
    VAR y = y + (x - ey) / ey

    VAR i = i + 1
  END

  y
END

FN log(base, x) THEN
  IF x <= 0 OR base <= 0 THEN 
    RETURN 0
  END 

  logn(x) / logn(base)
END 

FN sqrt(x) THEN
 x ^ 0.5
END

FN abs(x) THEN
  IF x < 0 THEN
    RETURN -x
  END

  x
END 

FN sign(x) THEN 
  IF x < 0 THEN 
    -1 
  ELIF x == 0 THEN 
    0
  ELSE 
    1
  END
END 

FN floor(x) THEN
  VAR int = to_int(x)

  IF (x < 0) AND (x != int) THEN 
    RETURN int - 1
  END 
  
  int
END 

FN fmod(x, y) THEN
  IF y == 0 THEN
    RETURN 0.0
  END 

  x - floor(x / y + 0.5) * y 
END 

FN sin(x) THEN
  VAR PI_2 = PI / 2.0
  VAR TWO_PI = PI * 2.0

  VAR x = fmod(x, TWO_PI)

  IF x > PI THEN
    VAR x = x - TWO_PI
  END

  IF x < -PI THEN
    VAR x = x + TWO_PI
  END

  IF x > PI_2 THEN
    VAR x = PI - x
  END

  IF x < -PI_2 THEN
    VAR x = -PI - x
  END

  VAR x2 = x * x

  x * (
    1
    - x2 * (
      1.0 / 6
      - x2 * (
        1.0 / 120
        - x2 * (
          1.0 / 5040
          - x2 * (1.0 / 362880)
        )
      )
    )
  )
END

FN cos(x) THEN 
  sin(x + PI / 2.0)
END 

FN tan(x) THEN
  sin(x) / cos(x)
END 

FN max(x) THEN 
  VAR type = typeof(x)
  VAR prev_max = 0
  VAR is_set = 0

  IF type == "list" THEN
    VAR len = len_of(x)
    VAR i = 0

    WHILE i < len THEN
      IF (x[i] > prev_max) OR (is_set == 0) THEN
        VAR prev_max = x[i]
        VAR is_set = 1
      END 

      VAR i = i + 1
    END 

    RETURN prev_max
  END 

  RuntimeError("Expected argument of type 'list', received '" + type + "'.")
END 

FN min(x) THEN 
  VAR type = typeof(x)
  VAR prev_min = 0
  VAR is_set = 0

  IF type == "list" THEN
    VAR len = len_of(x)
    VAR i = 0

    WHILE i < len THEN
      IF (x[i] < prev_min) OR (is_set == 0) THEN
        VAR prev_min = x[i]
        VAR is_set = 1
      END 

      VAR i = i + 1
    END 

    RETURN prev_min
  END 

  RuntimeError("Expected argument of type 'list', received '" + type + "'.")
END 
