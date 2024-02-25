#include <stdio.h>

#include <testing.h>

#include "list.h"
#include "dict.h"

void test_list_add_remove_add(void) {
    int* list = list_new(int);
    list_add(list, 1337);
    list_add(list, 4242);

    assert_equals(list_size(list), 2, "list size");
    assert_equals(list[0], 1337, "first element");
    assert_equals(list[1], 4242, "second element");

    list_remove(list, sizeof(int), 0);

    assert_equals(list_size(list), 1, "new list size");
    assert_equals(list[0], 4242, "new first element");
}

void test_dict_default(void) {
    dict_t* dict = dict_new();

    assert_null(dict_get(dict, "foo"), "new key does not exist");

    dict_put(dict, "foo", "bar");

    char* result = dict_get(dict, "foo");

    assert_not_null(result, "new key does exist");
    assert_streq("bar", result, "value matches");

    dict_free(dict);
}

int hash_string(const char*);
void test_dict_bug_duplicate_hash(void) {
    const char* key1 = "foo";
    const char* key2 = "bary";

    int hash1 = hash_string(key1) % NUMBER_OF_BUCKETS;
    int hash2 = hash_string(key2) % NUMBER_OF_BUCKETS;
    fprintf(stderr, "  hashes: %d, %d\n", hash1, hash2);
    assert_equals(hash1, hash2, "hashes don't match");

    dict_t* dict = dict_new();

    dict_put(dict, key1, "foo");

    char* result = dict_get(dict, key1);
    assert_not_null(result, "key1 doesn't exist");
    assert_streq("foo", result, "value1 doesn't match");

    dict_put(dict, key2, "bar");

    result = dict_get(dict, key2);
    assert_not_null(result, "key2 doesn't exist");
    fprintf(stderr, "  %s\n", result);
    assert_streq("bar", result, "value2 doesn't match");

    result = dict_get(dict, key1);
    assert_not_null(result, "key1 doesn't exist");
    assert_streq("foo", result, "value1 doesn't match");
}

int main(void) {
    test_init();

    test_run(test_list_add_remove_add);

    test_run(test_dict_default);
    test_run(test_dict_bug_duplicate_hash);

    if (test_results()) {
        return 0;
    } else {
        return 1;
    }
}
