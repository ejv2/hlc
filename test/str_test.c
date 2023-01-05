#include <stdio.h>
#include <string.h> /* heresy */

#define STR_AUTO_INCLUDE
#include "../str.h"

static const char *testptr = "Hello, world!\nabcdefghijklmnopqrstuvwxyz12345678910111213141516";
static const char *testptr2 = "";

void test_new()
{
	/* stock new string test */
	string_t str = str_new();
	size_t len = str_len(&str), cap = str_cap(&str);
	if (len != 0) {
		printf("expected new string to be zero length, got %lu\n", len);
		exit(1);
	}
	if (cap == 0) {
		printf("expected new string to be nonzero capacity, got %lu\n", len);
		exit(1);
	}
	str_free(&str);

	/* new string from existing cstring */
	string_t str2 = str_from(testptr);
	if (strlen(testptr) != str_len(&str2)) {
		printf("expected new string to have same length as original (orig: %lu, new: %lu)\n", strlen(testptr), str_len(&str2));
		exit(1);
	}
	printf("%s (cap: %ld)\n", str_cstr(&str2), str_cap(&str2));
	str_free(&str2);

	/* new string from existing *empty* cstring */
	string_t str3 = str_from(testptr2);
	if (strlen(testptr2) != str_len(&str3)) {
		printf("expected new string to have same length as original (orig: %lu, new: %lu)\n", strlen(testptr), str_len(&str3));
	}
	printf("%s (cap: %ld)\n", str_cstr(&str3), str_cap(&str3));
	str_free(&str3);
}

void test_trunc()
{
	/* truncation test */
	string_t str = str_from(testptr);
	str_truncate(&str, 3);
	if (str_len(&str) != 3) {
		printf("expected string to be truncated to three bytes, got %ld\n", str_len(&str));
		exit(1);
	}
	printf("%s (len: %ld)\n", str_cstr(&str), str_len(&str));
	str_free(&str);

	/* out of range truncation test */
	string_t str2 = str_new();
	size_t oldcap = str_cap(&str2);
	str_truncate(&str2, 3);
	if (str_len(&str2) != 0 || str_cap(&str2) != oldcap) {
		printf("bad length and/or cap modification (len: %ld, cap: %ld)\n", str_len(&str2), str_cap(&str2));
	}
	printf("empty: \"%s\" (%ld)\n", str_cstr(&str2), str_len(&str2));
	str_free(&str2);

	/* truncate to zero */
	string_t zstr = str_from(testptr);
	str_reset(&zstr);
	if (str_len(&zstr) != 0) {
		printf("expected string reset to zero length, got %ld\n", str_len(&zstr));
		exit(1);
	}
	str_free(&zstr);
}

void test_grow()
{
	string_t str;
	memset(&str, 0, sizeof(str));

	for (int i = 1; i <= 3; i++) {
		str_grow(&str, 5);
		const size_t expect = 5 * i;
		if (str_cap(&str) != expect) {
			printf("expected string from zero to have capacity %ld, got %ld\n", expect, str_cap(&str));
			exit(1);
		}
	}
}

void test_compact()
{
	string_t str = str_from(testptr);
	str_compact(&str);
	if (str_cap(&str) != str_len(&str)) {
		printf("expected capacity == len, got (len: %ld, cap: %ld)\n", str_len(&str), str_cap(&str));
		exit(1);
	}
	printf("compacted: \"%s\"\n", str_cstr(&str));
}

int main(void)
{
	test_new();
	test_trunc();
	test_grow();
	test_compact();
}
