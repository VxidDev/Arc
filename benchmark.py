import time
import sys
import json 

sys.setrecursionlimit(8096)

def benchmark(name, fn, iterations=1, print_res=True):
    start = time.perf_counter()
    for _ in range(iterations):
        result = fn()
    end = time.perf_counter()
    print(f"{name}: {(end - start) * 1000:.2f}ms (result: {result if print_res else 'skipped printing result'})")

# 1. Fibonacci (recursive) - tests function call overhead
def fib(n):
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)

benchmark("fibonacci(30) recursive", lambda: fib(30))

# 2. Fibonacci (iterative) - tests loop + arithmetic
def fib_iter(n):
    a, b = 0, 1
    i = 0
    while i < n:
        a, b = b, a + b
        i = i + 1
    return a

benchmark("fibonacci(1000) iterative", lambda: fib_iter(1000))

# 3. Sum of range - tests loop + integer arithmetic
def sum_range(n):
    s = 0
    i = 0
    while i < n:
        s = s + i
        i = i + 1
    return s

benchmark("sum 1..1000000", lambda: sum_range(1000000))

# 4. String concatenation - tests string handling
def str_concat(n):
    s = ""
    i = 0
    while i < n:
        s = s + "x"
        i = i + 1
    return len(s)

benchmark("string concat x10000", lambda: str_concat(10000))

# 5. List building - tests list/array handling
def build_list(n):
    lst = []
    i = 0
    while i < n:
        lst.append(i)
        i = i + 1
    return len(lst)

benchmark("list build x1000", lambda: build_list(1000))

# 6. Nested loops - tests loop overhead
def nested_loops(n):
    s = 0
    i = 0
    while i < n:
        j = 0
        while j < n:
            s = s + 1
            j = j + 1
        i = i + 1
    return s

benchmark("nested loops 1000x1000", lambda: nested_loops(1000))

# 7. Recursive countdown - tests tail-ish recursion
def countdown(n):
    if n <= 0:
        return 0
    return countdown(n - 1)

benchmark("recursive countdown 5000", lambda: countdown(5000))

# 8. Variable reassignment in loop - tests variable lookup
def var_churn(n):
    x = 0
    i = 0
    while i < n:
        x = x + i
        x = x * 1
        x = x - 0
        i = i + 1
    return x

benchmark("variable churn x100000", lambda: var_churn(100000))

json_string = None 

try:
    with open("nested.json", "r") as f:
        json_string = f.read()
except (FileNotFoundError, json.JSONDecodeError):
    print("nested.json not found, skipping benchmark.")

if json_string:
    benchmark("nested json parsing (3.7 mb)", lambda: json.loads(json_string), print_res = False)
