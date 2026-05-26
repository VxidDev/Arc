VAR PI = 3.141592653589793
VAR E = 2.718281828459045
VAR TAU = 6.283185307179586

FN exp(x) THEN 
  E ^ x
END 

FN logn(x) THEN
  IF x <= 0 THEN
    0
  ELSE
    VAR y = 0.0
    VAR i = 0

    WHILE i < 20 THEN
      VAR ey = exp(y)
      VAR y = y + (x - ey) / ey

      VAR i = i + 1
    END

    y
  END
END

FN log(base, x) THEN
  IF x <= 0 OR base <= 0 THEN 
    0
  ELSE 
    logn(x) / logn(base)
  END 

END 

FN sqrt(x) THEN
 x ^ 0.5
END

FN abs(x) THEN
  IF x < 0 THEN
    -x
  ELSE 
    x
  END
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

FN fmod(x, y) THEN
  IF y == 0 THEN
    0.0
  ELSE 
    x - truncate(x / y) * y
  END 
END 

FN sin(x) THEN
  VAR TWO_PI = PI * 2.0

  VAR x = fmod(x, TWO_PI)
  
  IF x > PI THEN
    VAR x = x - TWO_PI 
  END

  IF x < -PI THEN
    VAR x = x + TWO_PI 
  END 

  VAR x2 = x * x
  
  x * (1 - x2 * (1.0 / 6 - x2 * (1.0 / 120 - x2 * (1.0 / 5040))))
END 

FN cos(x) THEN 
  sin(x + PI / 2.0)
END 

FN tan(x) THEN
  sin(x) / cos(x)
END 


