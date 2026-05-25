FN exp(x) THEN 
  VAR sum = 1.0
  VAR term = 1.0

  VAR n = 1

  WHILE n < 30 THEN
    VAR term = term * (x / n)
    VAR sum = sum + term 

    VAR n = n + 1
  END 

  sum
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
