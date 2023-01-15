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

	str_free(&str);
}

void test_reserve()
{
	string_t str = str_from(testptr);
	size_t curlen = str_len(&str);
	size_t curcap = str_cap(&str);

	/*
	 * we need to ensure that we reserve for 5 extra characters.
	 * so, check how much more we need to reach cap and add 5 characters.
	 */
	size_t want = (curcap - curlen) + 5;
	str_reserve(&str, want);
	if (str_cap(&str) - curcap != 5) {
		printf("want delta cap = 5, got %lu (total: %lu)\n",
			str_cap(&str) - curcap,
			str_cap(&str));
		exit(1);
	}

	str_free(&str);
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

	str_free(&str);
}

void test_clone()
{
	string_t a = str_from(testptr);
	string_t b = str_clone(&a);

	a.s[0] = '0';
	if (a.s[0] == b.s[0]) {
		printf("expected cloned string to remain separate, both were modified");
	}
	printf("orig: %s\tclone: %s\n", str_cstr(&a), str_cstr(&b));

	str_free(&a);
	str_free(&b);
}

void test_equal()
{
	string_t a = str_from("string1");
	string_t b = str_from("string1");

	if (!str_equal(&a, &b)) {
		printf("expected `%s` and `%s` to be equal\n", str_cstr(&a), str_cstr(&b));
		exit(1);
	}

	string_t ncmp = str_from("different string");
	if (str_equal(&a, &ncmp)) {
		printf("expected `%s` and `%s` to *not* be equal\n", str_cstr(&a), str_cstr(&ncmp));
		exit(1);
	}

	str_free(&a);
	str_free(&b);
	str_free(&ncmp);
}

void test_compare()
{
	string_t a = str_from("abcdef");
	string_t b = str_from("zyx");

	if (str_compare(&a, &b) >= 0) {
		printf("lexicographic comparison of string with greater should return <0, got %d\n", str_compare(&a, &b));
		exit(1);
	}
	if (str_compare(&b, &a) <= 0) {
		printf("lexicographic comparison of string with lesser should return >0, got %d\n", str_compare(&b, &a));
		exit(1);
	}
	if (str_compare(&a, &a) != 0) {
		printf("lexicographic comparison of string with itself should return zero, got %d\n", str_compare(&a, &a));
		exit(1);
	}

	str_free(&a);
	str_free(&b);
}

void test_contains()
{
	string_t a = str_from("Hello, world!");

	if (!str_contains(&a, "ello")) {
		printf("expected a to contain `ello` (string: %s)\n", str_cstr(&a));
		exit(1);
	}

	if (!str_contains(&a, "ld!")) {
		printf("expected a to contain `ld!` (string: %s)\n", str_cstr(&a));
		exit(1);
	}

	if (!str_contains(&a, "H")) {
		printf("expected a to contain `H` (string: %s)\n", str_cstr(&a));
		exit(1);
	}

	if (str_contains(&a, "H!")) {
		printf("expected a *not* to contain `H!` (string: %s)\n", str_cstr(&a));
		exit(1);
	}

	str_free(&a);
}

void test_contains_char()
{
	string_t a = str_from("abcdefghijklmnopqrstuvwxyz");
	for (char c = 'a'; c <= 'z'; c++) {
		if (!str_contains_char(&a, c)) {
			printf("expected alphabet to contain all characters, got '%c' not matching\n", c);
			exit(1);
		}
	}

	str_free(&a);
}

void test_concat()
{
	string_t pref = str_from("Hello,"), suff = str_from(" world!");
	string_t concat = str_concat(&pref, &suff);

	if (strcmp("Hello, world!", str_cstr(&concat))) {
		printf("expected test string to match after concat, got \"%s\"\n", str_cstr(&concat));
		exit(1);
	}
	if (strlen(str_cstr(&concat)) != str_len(&concat)) {
		printf("probably missing nullbyte in str (strlen: %lu, len: %lu): \"%s\"\n",
				strlen(str_cstr(&concat)), str_len(&concat), str_cstr(&concat));

		char val = 0;
		for (unsigned int i = 0; !val; i++) {
			val = *(concat.e - i);
			printf("value of ->(e - %u): '%d'\n", i, val);

		}

		exit(1);
	}
	printf("single concat: \"%s\"\n", str_cstr(&concat));

	/* double concat */
	string_t dconcat = str_concat(&pref, &concat);
	if (strcmp("Hello,Hello, world!", str_cstr(&dconcat))) {
		printf("expected test string to match after double concat, got \"%s\"\n", str_cstr(&dconcat));
		exit(1);
	}
	printf("double concat: \"%s\"\n", str_cstr(&dconcat));

	/* long concat */
	string_t lstring = str_from("abcdefghijklmnopqrstuvwxyz012345678910111213141516");
	string_t lconcat = str_concat(&pref, &lstring);
	if (strcmp("Hello,abcdefghijklmnopqrstuvwxyz012345678910111213141516", str_cstr(&lconcat))) {
		for (const char *walk = lconcat.s; walk != lconcat.e; walk++) {
			printf("%d, ", *walk);
		}
		putchar('\n');

		printf("expected test string to match after double concat, got \"%s\"\n", str_cstr(&lconcat));
		exit(1);
	}
	printf("long concat: \"%s\"\n", str_cstr(&lconcat));

	/* null concat */
	string_t nulla, nullb;
	string_t app = str_from("sentinel");
	memset(&nulla, 0, sizeof(nulla));
	memset(&nullb, 0, sizeof(nullb));

	string_t na = str_concat(&nulla, &app);
	string_t nb = str_concat(&app, &nullb);
	if (strcmp("sentinel", str_cstr(&na))) {
		printf("expected null string to be ignored, got: \"%s\"", str_cstr(&na));
		exit(1);
	}
	if (strcmp("sentinel", str_cstr(&nb))) {
		printf("expected null string to be ignored, got: \"%s\"", str_cstr(&nb));
		exit(1);
	}
	printf("null concats: (%s, %s)\n", str_cstr(&na), str_cstr(&nb));

	str_free(&pref);
	str_free(&suff);
	str_free(&lstring);
	str_free(&concat);
	str_free(&dconcat);
	str_free(&lconcat);
	str_free(&app);
	str_free(&na);
	str_free(&nb);
}

void test_append()
{
	string_t a = str_from("prefix-");
	string_t b = str_from("suffix");

	str_append(&a, &b);
	if (strcmp(str_cstr(&a), "prefix-suffix")) {
		printf("expected 'prefix-suffix' from concatenation, got '%s'\n", str_cstr(&a));
		exit(1);
	}
	if (strlen(str_cstr(&a)) != str_len(&a)) {
		printf("probably missing nullbyte in str (strlen: %lu, len: %lu): \"%s\"\n",
				strlen(str_cstr(&a)), str_len(&a), str_cstr(&a));

		char val = 0;
		for (unsigned int i = 0; !val; i++) {
			val = *(a.e - i);
			printf("value of ->(e - %u): '%d'\n", i, val);
		}

		exit(1);
	}
	printf("append: %s\n", str_cstr(&a));

	string_t pref = str_from("alphabet+: ");
	string_t longstr = str_from("abcdefghijklmnopqrstuvwxyz12345678910111213141516!");
	str_append(&pref, &longstr);
	if (strcmp(str_cstr(&pref), "alphabet+: abcdefghijklmnopqrstuvwxyz12345678910111213141516!")) {
		printf("wrong result from concatenation, got '%s'\n", str_cstr(&pref));
		exit(1);
	}
	if (strlen(str_cstr(&pref)) != str_len(&pref)) {
		printf("probably missing nullbyte in str (strlen: %lu, len: %lu): \"%s\"\n",
				strlen(str_cstr(&pref)), str_len(&pref), str_cstr(&pref));

		char val = 0;
		for (unsigned int i = 0; !val; i++) {
			val = *(pref.e - i);
			printf("value of ->(e - %u): '%d'\n", i, val);
		}

		exit(1);
	}
	printf("long append: %s\n", str_cstr(&pref));

	str_free(&a);
	str_free(&b);
	str_free(&pref);
	str_free(&longstr);
}

int main(void)
{
	test_new();
	test_trunc();
	test_grow();
	test_reserve();
	test_compact();
	test_clone();
	test_equal();
	test_compare();
	test_contains();
	test_contains_char();
	test_concat();
	test_append();
}
