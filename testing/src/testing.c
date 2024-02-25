#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include "testing.h"

size_t tests_total;
size_t tests_succeeded;

bool success;

void test_init(void) {
    tests_total = 0;
    tests_succeeded = 0;
}

bool test_results(void) {
    fprintf(stderr, "\nTest Results:\n");
    fprintf(stderr, "%7zu total tests\n", tests_total);
    fprintf(stderr, "%7zu successful tests\n", tests_succeeded);
    fprintf(stderr, "%7zu failed tests\n", tests_total - tests_succeeded);
    fprintf(stderr, "\n");

    fprintf(stderr, "Overall Result: ");
    if (tests_total == tests_succeeded) {
        fprintf(stderr, "\033[32mSUCCESS\033[0m\n");
        return true;
    } else {
        fprintf(stderr, "\033[31mFAIL\033[0m\n");
        return false;
    }
}

void test_run_named(void (*test)(void), const char* test_name) {
    fprintf(stderr, "Test %s:\n", test_name);
    success = true;
    tests_total++;

    test();

    if (success) {
        fprintf(stderr, "  \033[32mSUCCESS\033[0m\n");
        tests_succeeded++;
    } else {
        fprintf(stderr, "  \033[31mFAIL\033[0m\n");
    }
}

void __assert(bool result, const char* message, const char* test_str, const char* file, const char* function, size_t lineno) {
    if (!result) {
        success = false;

        fprintf(stderr, "    ");
        if (message) {
            fprintf(stderr, "%s (%s)", message, test_str);
        } else {
            fprintf(stderr, "%s", test_str);
        }
        fprintf(stderr, " (%s, %s, %zu)\n", file, function, lineno);
    }
}
