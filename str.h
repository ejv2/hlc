/*
 * str.h - C99 implementation of a high-level string
 * Copyright (C) Ethan Marshall - 2023
 *
 * Requirements: stdlib.h, limits.h
 */

#ifdef STR_AUTO_INCLUDE
#include <stdlib.h>
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
static const char *str_cstr(const string_t *str)
{
	return str->s;
}
