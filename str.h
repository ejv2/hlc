/*
 * str.h - C99 implementation of a high-level string
 * Copyright (C) Ethan Marshall - 2023
 *
 * Requirements: stdlib.h, stdio.h, limits.h
 */

#ifdef HLC_AUTO_INCLUDE
#define STR_AUTO_INCLUDE
#endif

#ifdef STR_AUTO_INCLUDE
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#endif

/* Initial string buffer size allocated by str_new in bytes */
#define STR_INITIAL_BUFSIZ 32

/* The maximum size of a string in bytes */
#define STR_SIZE_MAX ((size_t)-1)

/*
 * string_t is a dynamically sized string with an associated length. Although
 * string_t emulates the style of a Pascal string, a null byte is still used to
 * terminate the buffer for bounds checking when using other standard C
 * routines or functions which expect null termination. The string buffer will
 * be stored on the heap.
 *
 * An empty string takes up one byte of space and has a length of zero. A
 * zero-initialized string_t is ready to use and is identical to one with a
 * zero length. The stored capacity (`cap`) is the number of characters that
 * may be stored excluding the NULL byte, meaning the cap is always one fewer
 * than the actual stored buffer size could store.
 *
 * Although this struct is not returned as an opaque handle, the modification
 * of its internals is to be discouraged, especially in the case of the cap
 * field, which must *always* be read through str_cap and str_grow.
 */
typedef struct {
	char *s, *e;
	size_t cap;
} string_t;

/*
 * str_new returns a new, ready to use string_t with zero length.
 * If buffer allocation fails, the string will still be valid and will remain
 * at zero length.
 */
static string_t str_new()
{
	char *buf = calloc(STR_INITIAL_BUFSIZ, sizeof(char));
	return (string_t){
		.s = buf,
		.e = buf,
		.cap = STR_INITIAL_BUFSIZ,
	};
}

/*
 * str_free frees all resources associated with a string and sets its length
 * and capacity to zero. A string can be used after a call to str_free, but
 * with a performance penalty due to re-allocation. If wishing to reset to
 * empty, use str_reset or str_truncate.
 */
static void str_free(string_t *str)
{
	free(str->s);
	str->e = str->s = NULL;
	str->cap = 0;
}

/*
 * str_cap returns the capacity of the string, which is how many characters can
 * be written before a reallocation occurs.
 */
static size_t str_cap(const string_t *str)
{
	return (str->cap == 0) ? 0 : str->cap - 1;
}

/*
 * str_len returns the length of the string, which is how many characters the
 * string currently holds. Note that this is not necessarily how many UTF-8
 * runes it holds, but how many bytes are in the string currently.
 */
static size_t str_len(const string_t *str)
{
	/* need to subtract one if non-zero, as the null byte does not count */
	return str->e - str->s;
}

/*
 * str_grow grows the string by delta characters, meaning delta more characters
 * can be written with no reallocation. If delta is zero, str_grow will double
 * the current capacity. If delta would overflow the maximum string capacity,
 * the maximum string capacity is used instead.
 *
 * If allocation fails or the maximum string capacity was reached, zero is
 * returned, else one.
 */
static int str_grow(string_t *str, size_t delta)
{
	int stat = 1;
	size_t oldlen = str->e - str->s;
	size_t newcap = str->cap + delta;

	/*
	 * Workaround for unintuitive behaviour on zero start.
	 * One byte extra is needed for NULL, so a zero start causes one byte
	 * LESS for capacity on first allocation.
	 */
	if (str->cap == 0)
		newcap++;

	if (STR_SIZE_MAX - delta < str->cap) {
		newcap = STR_SIZE_MAX;
		stat = 0;
	}

	if (delta == 0) {
		if (STR_SIZE_MAX - str->cap < str->cap) {
			newcap = STR_SIZE_MAX;
			stat = 0;
		}
		newcap = str->cap * 2;
	}

	str->s = realloc(str->s, sizeof(char) * newcap);
	str->e = str->s + oldlen;
	str->cap = newcap;

	return stat;
}

/*
 * str_reserve ensures that string str can store at least delta more characters
 * with no reallocation, but does not grow the string if the capacity is
 * already sufficient. Returns -1 if the capacity was already sufficient.
 * Returns zero if reallocation was attempted but failed and returns one on
 * success.
 */
static int str_reserve(string_t *str, size_t delta)
{
	size_t cap = str_cap(str);
	size_t len = str_len(str);

	if (cap - len >= delta) {
		return -1;
	}

	return str_grow(str, delta);
}

/*
 * str_compact shrinks the string to the minimum size required to store all
 * data in the string. This is useful after all processing is complete and a
 * string is to be stored, but will cause poorer than ideal performance if
 * called before any operations which cause a reallocation. If the reallcation
 * fails, the function becomes a no-op.
 */
static void str_compact(string_t *str)
{
	size_t len = str->e - str->s;
	char *buf = realloc(str->s, len + 1);
	if (!buf)
		return;

	str->cap = len + 1;
	str->s = buf;
	str->e = str->s + len;
}

/*
 * str_truncate truncates the end of the string such that the string becomes
 * len in length. The string capacity is unaffected. If len is out of range, no
 * operation is performed.
 */
static void str_truncate(string_t *str, size_t len)
{
	if (len >= (size_t)(str->e - str->s)) {
		return;
	}

	str->e = str->s + len;
	*str->e = '\0';
}

/*
 * str_reset resets the string to and empty string, but does not change the
 * capacity.
 */
static void str_reset(string_t *str)
{
	str_truncate(str, 0);
}

/*
 * str_from returns a new string_t initialized with the null terminated C
 * string cstr. If cstr is NULL, str_from() is equivalent to str_new(). If
 * buffer allocation fails, the string will still be valid, but will be
 * truncated to an empty string. cstr need not remain valid after str_from has
 * completed and may be discarded.
 */
static string_t str_from(const char *cstr)
{
	const char *walk = cstr;
	string_t ret = str_new();

	if (!cstr)
		return ret;

	for (;;) {
		*ret.e = *walk;
		if (*walk++ == '\0')
			break;
		ret.e++;

		/*
		 * note: allowing the overwrite of the NULL here, as we intend
		 * to reallocate anyway
		 */
		if ((size_t)(ret.e - ret.s) == ret.cap) {
			if (!str_grow(&ret, 0)) {
				str_free(&ret);
				return str_new();
			}
		}
	}

	return ret;
}

/*
 * str_cstr returns the contents of the string as a C string, compatible with
 * all standard library string functions. The string returned is a const to
 * discourage the direct modification of the string and reduce memory errors.
 *
 * The results of this function may be invalid after any call which modifies
 * the string. *Never* save the results of this function.
 */
static char *str_cstr(const string_t *str)
{
	return str->s;
}

/*
 * str_clone constructs and returns a fresh copy of str. A new heap block is
 * allocated to store the new copy, as in strdup, unless the string has zero
 * length, in which case no allocation takes place.
 */
static string_t str_clone(const string_t *str)
{
	if (str_len(str) == 0) {
		return (string_t){NULL, NULL, 0};
	}
	return str_from(str_cstr(str));
}

/*
 * str_concat returns a newly allocated string which stores the concatenation
 * of a and b. If allocation fails or the value would exceed the maximum
 * capacity, an empty string is returned.
 */
static string_t str_concat(const string_t *a, const string_t *b)
{
	string_t work = str_new();
	if (!str_grow(&work, str_len(a) + str_len(b)))
		return work;

	const char *walk = a->s;
	if (walk) {
		do {
			*work.e = *walk;
			walk++, work.e++;
		} while (walk < a->e);
	}

	walk = b->s;
	if (walk) {
		do {
			*work.e = *walk;
			walk++, work.e++;
		} while (walk < b->e);
	}

	*work.e = '\0';
	return work;
}

/*
 * str_get returns the ith character from string s. If i is out of range,
 * str_get calls abort with a failure message.
 */
static char str_get(string_t *s, size_t i)
{
	if (i > str_len(s)) {
		printf("PANIC: string index out of range (i: %lu, len: %lu)\n", i, str_len(s));
		abort();
	}

	return s->s[i];
}

/*
 * str_set sets the byte at index i to the value c. If i is out of range,
 * str_set calls abort with a failure message.
 */
static void str_set(string_t *s, size_t i, char c)
{
	if (i > str_len(s)) {
		printf("PANIC: string index out of range (i: %lu, len: %lu)\n", i, str_len(s));
		abort();
	}

	s->s[i] = c;
}

/*
 * str_append appends src to dst in place, guaranteeing that only a single
 * (re)allocation may take place.
 */
static void str_append(string_t *dst, const string_t *src)
{
	if (!src)
		return;
	if (!str_reserve(dst, str_len(src)))
		return;

	for (const char *walk = src->s; walk < src->e; walk++) {
		*(dst->e++) = *walk;
	}
	*dst->e = '\0';
	return;
}

/*
 * str_equal returns true (>0) if string a and b are the same length and
 * contain the same text, else returns false (0).
 */
static int str_equal(const string_t *a, const string_t *b)
{
	if (str_len(a) != str_len(b))
		return 0;

	for (const char *walk = a->s; walk != a->e; walk++) {
		size_t i = (size_t)(walk - a->s);
		if (*walk != b->s[i])
			return 0;
	}

	return 1;
}

/*
 * str_compare lexicographically compares a and b using the same method as the
 * standard library's strcmp, but more efficiently for this style of string. If
 * string a, when sorted lexicographically, would come after string b, the
 * returned value is greater than zero. If the strings are equal, zero is
 * returned. Else, a negative  integer is returned.
 */
static int str_compare(const string_t *a, const string_t *b)
{
	const char *wa = a->s, *wb = b->s;
	for (; *wa==*wb && *wa; wa++, wb++);
	return *(unsigned char *)wa - *(unsigned char *)wb;
}

/*
 * str_contains returns true if the string str contains the substring substr at
 * any position.
 */
static int str_contains(const string_t *str, const char *substr)
{
	const char *walk, *subwalk = substr;
	for (walk = str->s; walk <= str->e; walk++) {
		/* *subwalk is null byte, we walked the whole substring, so match */
		if (*subwalk == '\0') {
			return 1;
		}

		/* not a match, reset subwalk */
		if (*walk != *subwalk) {
			subwalk = substr;
			continue;
		}

		/* match; increment subwalk along with walk */
		subwalk++;
	}

	/* reached end with no return; no match */
	return 0;
}

/*
 * str_contains_char returns true (>0) if string str contains the character c,
 * else returns false (0).
 */
static int str_contains_char(const string_t *str, char c)
{
	for (const char *walk = str->s; walk != str->e; walk++) {
		if (*walk == c)
			return 1;
	}

	return 0;
}
