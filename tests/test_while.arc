IMPORT "__sys"

VAR i = 0

WHILE i < 1000000 THEN # huge loop
  i = i + 1
END 

i = 0

WHILE i > 1 THEN # shouldn't execute 
  exit(1)
END 
