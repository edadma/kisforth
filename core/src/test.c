#include "forth.h"
#include <string.h>
#include <stdio.h>

#ifdef FORTH_ENABLE_TESTS

// Global test statistics
test_stats_t test_stats = {0, 0, 0, NULL};

#define TEST_DIVISION(name, div_word, dividend_lo, dividend_hi, divisor, expected_remainder, expected_quotient) \
    do { \
        forth_reset(); \
        data_push(dividend_lo); \
        data_push(dividend_hi); \
        data_push(divisor); \
        execute_word(div_word); \
        test_stats.total++; \
        if (data_depth() == 2) { \
            cell_t actual_quotient = data_pop(); \
            cell_t actual_remainder = data_pop(); \
            if (actual_quotient == (expected_quotient) && actual_remainder == (expected_remainder)) { \
                test_stats.passed++; \
            } else { \
                test_stats.failed++; \
                printf("    FAIL %s: expected rem=%d quo=%d, got rem=%d quo=%d\n", \
                       name, (int)(expected_remainder), (int)(expected_quotient), \
                       (int)actual_remainder, (int)actual_quotient); \
            } \
        } else { \
            test_stats.failed++; \
            printf("    FAIL %s: wrong stack depth %d (expected 2)\n", name, data_depth()); \
        } \
    } while(0)

// Complete system reset for test isolation
void forth_reset(void) {
    // Clear memory and reset HERE
    here = 0;
    memset(forth_memory, 0, FORTH_MEMORY_SIZE);

    // Clear stacks
    stack_init();

    input_system_init();

    // Clear input buffer
    set_input_buffer("");

    // Rebuild primitive dictionary
    dictionary_init();
    create_all_primitives();
}

// Execute Forth code and check results
bool test_forth_code(const char* code, cell_t expected_top, int expected_depth) {
    // Complete system reset for test isolation
    forth_reset();

    test_stats.total++;

    // Execute the test code
    set_input_buffer(code);
    interpret();

    // Check stack state
    bool depth_ok = (data_depth() == expected_depth);
    bool top_ok = (expected_depth == 0) || (data_peek() == expected_top);

    if (depth_ok && top_ok) {
        test_stats.passed++;
        return true;
    } else {
        test_stats.failed++;
        printf("\n    FAIL: expected depth=%d", expected_depth);
        if (expected_depth > 0) {
            printf(", top=%d", expected_top);
        }
        printf("; got depth=%d", data_depth());
        if (data_depth() > 0) {
            printf(", top=%d", data_peek());
        }
        printf("\n");
        return false;
    }
}

// Individual test functions
static void test_memory_functions(void) {
    forth_addr_t old_here = here;
    forth_addr_t addr = forth_allot(100);

    TEST_ASSERT_TRUE(addr < FORTH_MEMORY_SIZE);
    TEST_ASSERT_EQUAL(old_here, addr);
    TEST_ASSERT_EQUAL(here, addr + 100);

    forth_align();
    TEST_ASSERT_TRUE(here % sizeof(cell_t) == 0);
}

static void test_stack_functions(void) {
    stack_init();
    TEST_ASSERT_STACK_DEPTH(0);

    data_push(42);
    TEST_ASSERT_STACK_DEPTH(1);
    TEST_ASSERT_STACK_TOP(42);

    data_push(100);
    TEST_ASSERT_STACK_DEPTH(2);
    TEST_ASSERT_STACK_TOP(100);

    cell_t value = data_pop();
    TEST_ASSERT_EQUAL(100, value);
    TEST_ASSERT_STACK_DEPTH(1);
    TEST_ASSERT_STACK_TOP(42);
}

static void test_dictionary_functions(void) {
    word_t* plus_word = find_word("+");
    TEST_ASSERT_NOT_NULL(plus_word);
    TEST_ASSERT_TRUE(strcmp(plus_word->name, "+") == 0);
    TEST_ASSERT_NOT_NULL(plus_word->cfunc);

    word_t* nonexistent = find_word("NONEXISTENT");
    TEST_ASSERT_TRUE(nonexistent == NULL);
}

static void test_division_functions(void) {
    // Basic functional tests using direct C calls
    word_t* sm_rem = find_word("SM/REM");
    word_t* fm_mod = find_word("FM/MOD");
    TEST_ASSERT_NOT_NULL(sm_rem);
    TEST_ASSERT_NOT_NULL(fm_mod);

    // Test SM/REM basic case: 10 ÷ 7 = 1 remainder 3
    forth_reset();
    data_push(10); data_push(0); data_push(7);
    execute_word(sm_rem);
    TEST_ASSERT_STACK_DEPTH(2);
    TEST_ASSERT_EQUAL(1, data_pop());  // quotient
    TEST_ASSERT_EQUAL(3, data_pop());  // remainder

    // Test FM/MOD different result: -10 ÷ 7 = -2 remainder 4 (floored)
    forth_reset();
    data_push(-10); data_push(-1); data_push(7);
    execute_word(fm_mod);
    TEST_ASSERT_STACK_DEPTH(2);
    TEST_ASSERT_EQUAL(-2, data_pop()); // quotient (floored)
    TEST_ASSERT_EQUAL(4, data_pop());  // remainder (sign of divisor)
}

static void test_division_comprehensive(void) {
    word_t* sm_rem = find_word("SM/REM");
    word_t* fm_mod = find_word("FM/MOD");
    TEST_ASSERT_NOT_NULL(sm_rem);
    TEST_ASSERT_NOT_NULL(fm_mod);

    // SM/REM tests (Symmetric Division - remainder has sign of dividend)
    TEST_DIVISION("SM/REM 10÷7", sm_rem, 10, 0, 7, 3, 1);
    TEST_DIVISION("SM/REM -10÷7", sm_rem, -10, -1, 7, -3, -1);
    TEST_DIVISION("SM/REM 10÷-7", sm_rem, 10, 0, -7, 3, -1);
    TEST_DIVISION("SM/REM -10÷-7", sm_rem, -10, -1, -7, -3, 1);

    // FM/MOD tests (Floored Division - remainder has sign of divisor)
    TEST_DIVISION("FM/MOD 10÷7", fm_mod, 10, 0, 7, 3, 1);
    TEST_DIVISION("FM/MOD -10÷7", fm_mod, -10, -1, 7, 4, -2);
    TEST_DIVISION("FM/MOD 10÷-7", fm_mod, 10, 0, -7, -4, -2);
    TEST_DIVISION("FM/MOD -10÷-7", fm_mod, -10, -1, -7, -3, 1);

    // Edge cases
    TEST_DIVISION("SM/REM 0÷5", sm_rem, 0, 0, 5, 0, 0);
    TEST_DIVISION("FM/MOD 7÷7", fm_mod, 7, 0, 7, 0, 1);

    // Verify mathematical identity: divisor * quotient + remainder = dividend
    test_stats.current_test_name = "Division Identity Check";
    forth_reset();
    data_push(-10); data_push(-1); data_push(7);
    execute_word(sm_rem);
    cell_t q = data_pop(); cell_t r = data_pop();
    TEST_ASSERT_EQUAL(-10, 7 * q + r);  // 7 * (-1) + (-3) = -10 ✓
}

// Main test runner
void run_all_tests(void) {
    test_stats = (test_stats_t){0, 0, 0, NULL};

    printf("Running KISForth Unit Tests...\n\n");

    // C function tests
    TEST_FUNC("Memory Management", test_memory_functions);
    TEST_FUNC("Stack Operations", test_stack_functions);
    TEST_FUNC("Dictionary Lookup", test_dictionary_functions);
    TEST_FUNC("Division Functions", test_division_functions);
    TEST_FUNC("Division Comprehensive", test_division_comprehensive);

    // Forth code tests
    TEST_FORTH("Basic Addition", "10 20 +", 30, 1);
    TEST_FORTH("Subtraction", "50 8 -", 42, 1);
    TEST_FORTH("Multiplication", "6 7 *", 42, 1);
    TEST_FORTH("Division", "84 2 /", 42, 1);
    TEST_FORTH("Empty Stack", "42 DROP", 0, 0);
    TEST_FORTH("Complex Expression", "2 3 + 4 *", 20, 1);
    TEST_FORTH("Multiple Operations", "10 5 / 3 + 2 *", 10, 1);
    TEST_FORTH("Multiple Results", "100 25 - 30 10 +", 40, 2);

    TEST_FORTH("SM/REM Basic", "10 0 7 SM/REM", 1, 2);      // quotient on top
    TEST_FORTH("FM/MOD Basic", "10 0 7 FM/MOD", 1, 2);      // quotient on top

    // Test SOURCE and >IN behavior
    test_stats.current_test_name = "Input Buffer Functions";
    set_input_buffer("123 456");
    TEST_ASSERT_EQUAL(7, get_current_input_length());
    TEST_ASSERT_TRUE(get_current_input_buffer_addr() < FORTH_MEMORY_SIZE);
    TEST_ASSERT_EQUAL(0, get_current_to_in());

    // Final summary
    printf("\n" "=" "=" "=" "=" "=" " Test Summary " "=" "=" "=" "=" "=" "\n");
    printf("Passed: %d\n", test_stats.passed);
    printf("Failed: %d\n", test_stats.failed);
    printf("Total:  %d\n", test_stats.total);

    if (test_stats.failed == 0) {
        printf("🎉 All tests passed!\n");
    } else {
        printf("❌ %d test(s) failed\n", test_stats.failed);
    }

    forth_reset();
    printf("\n");
}

// TEST word implementation
void f_test(word_t* self) {
    (void)self;

    run_all_tests();
}

#else

// Stubs when tests disabled
void forth_reset(void) {
    // Still need basic reset functionality
    here = 0;
    memset(forth_memory, 0, FORTH_MEMORY_SIZE);
    stack_init();
    set_input_buffer("");
    dictionary_init();
    create_all_primitives();
}

#endif