#ifndef TESTING_H
#define TESTING_H

#include <stdbool.h>
#include <string.h>

void test_init(void);
void test_run_named(void (*)(void), const char*);
bool test_results(void);

#define test_run(f) test_run_named(f, # f)

void __assert(bool, const char*, const char*, const char*, const char*, size_t);

#define _assert(b, m, t) { bool r = b; __assert(r, m, t, __FILE__, __FUNCTION__, __LINE__); if (!r) return; }
#define assert_true(b, m) _assert(b, m, # b)
#define assert_false(b, m) assert_true(b, m)
#define assert_equals(e, a, m) assert_true(e == a, m)
#define assert_null(a, m) assert_true(NULL == a, m)
#define assert_not_null(a, m) assert_true(NULL != a, m)
#define assert_strlen(s, l, m) assert_true(strlen(s) == l, m)
#define assert_streq(s1, s2, m) _assert(strcmp(s1, s2) == 0, m, # s1 " == " # s2)
#define assert_array_equals(a1, a2, l, m) { for(size_t _i = 0; _i < l; _i++) { assert_equals(a1[i], a2[i], m); } }

#endif
