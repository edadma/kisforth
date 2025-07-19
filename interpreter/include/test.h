#ifndef TEST_H
#define TEST_H

#include <stdbool.h>



// System reset (always available)
void forth_reset(void);

#ifdef FORTH_ENABLE_TESTS

// Test statistics structure
typedef struct {
  int passed;
  int failed;
  int total;
  const char* current_test_name;
} test_stats_t;

extern test_stats_t test_stats;

// Test functions
bool test_forth_code(const char* code, cell_t expected_top, int expected_depth);
void run_all_tests(void);
void f_test(word_t* self);  // Forth TEST primitive

void create_test_primitives(void);

// Test assertion macros
#define TEST_ASSERT_TRUE(condition)                                      \
  do {                                                                   \
    test_stats.total++;                                                  \
    if (condition) {                                                     \
      test_stats.passed++;                                               \
    } else {                                                             \
      test_stats.failed++;                                               \
      printf("    FAIL: %s (line %d) in %s\n", #condition, __LINE__,     \
             test_stats.current_test_name ? test_stats.current_test_name \
                                          : "unknown");                  \
    }                                                                    \
  } while (0)

#define TEST_ASSERT_EQUAL(expected, actual)                              \
  do {                                                                   \
    test_stats.total++;                                                  \
    if ((expected) == (actual)) {                                        \
      test_stats.passed++;                                               \
    } else {                                                             \
      test_stats.failed++;                                               \
      printf("    FAIL: expected %d, got %d (line %d) in %s\n",          \
             (int)(expected), (int)(actual), __LINE__,                   \
             test_stats.current_test_name ? test_stats.current_test_name \
                                          : "unknown");                  \
    }                                                                    \
  } while (0)

#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT_TRUE((ptr) != NULL)

#define TEST_ASSERT_STACK_DEPTH(expected) \
  TEST_ASSERT_EQUAL(expected, data_depth())

#define TEST_ASSERT_STACK_TOP(expected) TEST_ASSERT_EQUAL(expected, data_peek())

#define TEST_FORTH(name, code, expected_top, expected_depth)               \
  do {                                                                     \
    printf("  %s: \"%s\"", name, code);                                    \
    int old_failed = test_stats.failed;                                    \
    test_stats.current_test_name = name;                                   \
    test_forth_code(code, expected_top, expected_depth);                   \
    printf(" -> %s\n", test_stats.failed == old_failed ? "PASS" : "FAIL"); \
  } while (0)

#define TEST_FUNC(name, func)                                              \
  do {                                                                     \
    printf("  %s", name);                                                  \
    int old_failed = test_stats.failed;                                    \
    test_stats.current_test_name = name;                                   \
    func();                                                                \
    printf(" -> %s\n", test_stats.failed == old_failed ? "PASS" : "FAIL"); \
  } while (0)

#else

// Stubs when tests disabled
#define TEST_ASSERT_TRUE(condition) \
  do {                              \
  } while (0)
#define TEST_ASSERT_EQUAL(expected, actual) \
  do {                                      \
  } while (0)
#define TEST_ASSERT_NOT_NULL(ptr) \
  do {                            \
  } while (0)
#define TEST_ASSERT_STACK_DEPTH(expected) \
  do {                                    \
  } while (0)
#define TEST_ASSERT_STACK_TOP(expected) \
  do {                                  \
  } while (0)
#define TEST_FORTH(name, code, expected_top, expected_depth) \
  do {                                                       \
  } while (0)
#define TEST_FUNC(name, func) \
  do {                        \
  } while (0)

// Stub functions
static inline void run_all_tests(void) {
  printf("Tests not compiled in. Rebuild with -DENABLE_TESTS=ON\n");
}

static inline void f_test(word_t* self) {
  (void)self;
  printf("Tests not compiled in. Rebuild with -DENABLE_TESTS=ON\n");
}

#endif  // FORTH_ENABLE_TESTS

#endif  // TEST_H