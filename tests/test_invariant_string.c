#include <check.h>
#include <stdlib.h>
#include <stdint.h>
#include "../src/objects/string.h"

START_TEST(test_mulString_no_overflow)
{
    // Invariant: mulString must never allocate a buffer smaller than needed bytes
    const struct {
        size_t initial_len;
        int64_t times;
    } payloads[] = {
        {SIZE_MAX / 2 + 1, 2},           // Exact overflow case: (SIZE_MAX/2+1)*2 wraps to 0
        {65537, SIZE_MAX / 65536 + 1},   // Boundary overflow: 65537 * (SIZE_MAX/65536+1) overflows
        {10, 5},                         // Valid input: 10 * 5 = 50
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        String *str = initString("A", payloads[i].initial_len);
        Number num;
        num.as.i = payloads[i].times;
        
        String *result = mulString(str, &num);
        
        if (result) {
            // If multiplication succeeds, verify length matches expected product
            // without overflow
            size_t expected;
            if (__builtin_mul_overflow(payloads[i].initial_len, 
                                       (size_t)payloads[i].times, 
                                       &expected)) {
                // Overflow occurred - result should be NULL or empty string
                ck_assert_msg(result->len == 0 || result == NULL, 
                    "mulString should handle overflow safely");
            } else {
                // No overflow - verify correct length
                ck_assert_msg(result->len == expected, 
                    "mulString should compute correct length without overflow");
            }
            freeString(result);
        }
        freeString(str);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_mulString_no_overflow);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}