IMPORT "@stdlib/assert.arc"
IMPORT "__sys"

# --- Test 1: Basic FOR loop over a list ---
# We loop over [1, 2, 3, 4, 5] and add them up.
# The sum should be 1+2+3+4+5 = 15

VAR sum = 0
VAR numbers = [1, 2, 3, 4, 5]

FOR n IN numbers THEN
    sum = sum + n
END

assert_eq(sum, 15, "for loop sum over list")

# --- Test 2: FOR loop over a string ---
# Looping over a string goes character by character.
# "hello" has 5 characters, so count should be 5.

VAR count = 0
FOR ch IN "hello" THEN
    count = count + 1
END

assert_eq(count, 5, "for loop over string counts characters")

# --- Test 3: BREAK stops the loop early ---
# We loop over [10, 20, 30, 40].
# When we hit 30 we BREAK, so broke_at should be 20 (last value before break).

VAR broke_at = 0
FOR n IN [10, 20, 30, 40] THEN
    IF n == 30 THEN
        BREAK
    END
    broke_at = n
END

assert_eq(broke_at, 20, "break stops for loop at right time")

# --- Test 4: CONTINUE skips one item ---
# We loop over [1, 2, 3, 4, 5].
# When n == 3, we CONTINUE (skip it), so collected = 1+2+4+5 = 12.

VAR collected = 0
FOR n IN [1, 2, 3, 4, 5] THEN
    IF n == 3 THEN
        CONTINUE
    END
    collected = collected + n
END

assert_eq(collected, 12, "continue skips item 3 (1+2+4+5=12)")

# --- Test 5: FOR loop over an empty list ---
# Nothing should happen — ran should stay 0.

VAR ran = 0
FOR n IN [] THEN
    ran = ran + 1
END

assert_eq(ran, 0, "for loop over empty list does not execute body")

print("test_for_loops.arc passed\n")
