#!/bin/bash
# test_warnings.sh — verify compiler warnings fire correctly
# Each test checks that a specific warning text appears in stderr

ARC=./arc
PASS=0
FAIL=0

check_warning() {
  local label="$1"
  local code="$2"
  local expected="$3"

  tmpfile=$(mktemp /tmp/test_warn_XXXXXX.arc)
  printf '%s\n' "$code" > "$tmpfile"
  output=$($ARC "$tmpfile" 2>&1)
  rc=$?
  rm -f "$tmpfile"

  if echo "$output" | grep -q "$expected"; then
    printf "  \033[32mPASS\033[0m  %s\n" "$label"
    PASS=$((PASS+1))
  else
    printf "  \033[31mFAIL\033[0m  %s\n" "$label"
    echo "    expected warning containing: $expected"
    echo "    got: $output"
    FAIL=$((FAIL+1))
  fi
}

check_no_warning() {
  local label="$1"
  local code="$2"
  local not_expected="$3"

  tmpfile=$(mktemp /tmp/test_warn_XXXXXX.arc)
  printf '%s\n' "$code" > "$tmpfile"
  output=$($ARC "$tmpfile" 2>&1)
  rc=$?
  rm -f "$tmpfile"

  if echo "$output" | grep -q "$not_expected"; then
    printf "  \033[31mFAIL\033[0m  %s\n" "$label"
    echo "    unexpected warning: $not_expected"
    echo "    got: $output"
    FAIL=$((FAIL+1))
  elif [ $rc -ne 0 ]; then
    printf "  \033[31mFAIL\033[0m  %s\n" "$label"
    echo "    program failed with exit $rc"
    echo "    output: $output"
    FAIL=$((FAIL+1))
  else
    printf "  \033[32mPASS\033[0m  %s\n" "$label"
    PASS=$((PASS+1))
  fi
}

echo "Testing compiler warnings:"
echo ""

# warnings that should fire
check_warning \
  "BREAK outside loop warns" \
  "BREAK" \
  "'BREAK' used outside of a loop"

check_warning \
  "CONTINUE outside loop warns" \
  "CONTINUE" \
  "'CONTINUE' used outside of a loop"

check_warning \
  "RETURN outside function warns" \
  "RETURN 42" \
  "'RETURN' used outside of a function"

# warnings that should NOT fire
check_no_warning \
  "BREAK inside loop: no warning" \
  'VAR i = 0
WHILE i < 5 THEN
  BREAK
  i = i + 1
END
print("ok")
' \
  "used outside"

check_no_warning \
  "CONTINUE inside loop: no warning" \
  'VAR i = 0
WHILE i < 5 THEN
  i = i + 1
  IF i == 3 THEN
    CONTINUE
  END
END
print("ok")
' \
  "used outside"

check_no_warning \
  "RETURN inside function: no warning" \
  'FN foo() THEN
  RETURN 1
END
print(foo())
' \
  "used outside"

echo ""
printf "  %d passed, %d failed\n" "$PASS" "$FAIL"
echo ""

if [ $FAIL -ne 0 ]; then
  exit 1
fi
