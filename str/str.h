/*
 * str.h - C99 implementation of a high-level string
 * Copyright (C) Ethan Marshall - 2023
 *
 * NOTE: This is a source-header library. You must both compile the
 * corresponding source file and include this header.
 */

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
string_t str_new();

/*
 * str_free frees all resources associated with a string and sets its length
 * and capacity to zero. A string can be used after a call to str_free, but
 * with a performance penalty due to re-allocation. If wishing to reset to
 * empty, use str_reset or str_truncate.
 */
void str_free(string_t *str);

/*
 * str_cap returns the capacity of the string, which is how many characters can
 * be written before a reallocation occurs.
 */
size_t str_cap(const string_t *str);

/*
 * str_len returns the length of the string, which is how many characters the
 * string currently holds. Note that this is not necessarily how many UTF-8
 * runes it holds, but how many bytes are in the string currently.
 */
size_t str_len(const string_t *str);

/*
 * str_grow grows the string by delta characters, meaning delta more characters
 * can be written with no reallocation. If delta is zero, str_grow will double
 * the current capacity. If delta would overflow the maximum string capacity,
 * the maximum string capacity is used instead.
 *
 * If allocation fails or the maximum string capacity was reached, zero is
 * returned, else one.
 */
int str_grow(string_t *str, size_t delta);

/*
 * str_reserve ensures that string str can store at least delta more characters
 * with no reallocation, but does not grow the string if the capacity is
 * already sufficient. Returns -1 if the capacity was already sufficient.
 * Returns zero if reallocation was attempted but failed and returns one on
 * success.
 */
int str_reserve(string_t *str, size_t delta);

/*
 * str_compact shrinks the string to the minimum size required to store all
 * data in the string. This is useful after all processing is complete and a
 * string is to be stored, but will cause poorer than ideal performance if
 * called before any operations which cause a reallocation. If the reallcation
 * fails, the function becomes a no-op.
 */
void str_compact(string_t *str);

/*
 * str_truncate truncates the end of the string such that the string becomes
 * len in length. The string capacity is unaffected. If len is out of range, no
 * operation is performed.
 */
void str_truncate(string_t *str, size_t len);

/*
 * str_reset resets the string to and empty string, but does not change the
 * capacity.
 */
void str_reset(string_t *str);

/*
 * str_from returns a new string_t initialized with the null terminated C
 * string cstr. If cstr is NULL, str_from() is equivalent to str_new(). If
 * buffer allocation fails, the string will still be valid, but will be
 * truncated to an empty string. cstr need not remain valid after str_from has
 * completed and may be discarded.
 */
string_t str_from(const char *cstr);

/*
 * str_cstr returns the contents of the string as a C string, compatible with
 * all standard library string functions. The string returned is a const to
 * discourage the direct modification of the string and reduce memory errors.
 *
 * The results of this function may be invalid after any call which modifies
 * the string. *Never* save the results of this function.
 */
char *str_cstr(const string_t *str);

/*
 * str_clone constructs and returns a fresh copy of str. A new heap block is
 * allocated to store the new copy, as in strdup, unless the string has zero
 * length, in which case no allocation takes place.
 */
string_t str_clone(const string_t *str);

/*
 * str_concat returns a newly allocated string which stores the concatenation
 * of a and b. If allocation fails or the value would exceed the maximum
 * capacity, an empty string is returned.
 */
string_t str_concat(const string_t *a, const string_t *b);

/*
 * str_get returns the ith character from string s. If i is out of range,
 * str_get calls abort with a failure message.
 */
char str_get(string_t *s, size_t i);

/*
 * str_set sets the byte at index i to the value c. If i is out of range,
 * str_set calls abort with a failure message.
 */
void str_set(string_t *s, size_t i, char c);

/*
 * str_append appends src to dst in place, guaranteeing that only a single
 * (re)allocation may take place.
 */
void str_append(string_t *dst, const string_t *src);

/*
 * str_fmt formats a string as though it were returned by sprintf, guaranteeing
 * that only a single allocation may take place and that the resulting data
 * cannot overflow the stack. If allocation fails or fmt contains a syntax
 * error, an empty string is returned.
 */
string_t str_fmt(const char *fmt, ...);

/*
 * str_equal returns true (>0) if string a and b are the same length and
 * contain the same text, else returns false (0).
 */
int str_equal(const string_t *a, const string_t *b);

/*
 * str_compare lexicographically compares a and b using the same method as the
 * standard library's strcmp, but more efficiently for this style of string. If
 * string a, when sorted lexicographically, would come after string b, the
 * returned value is greater than zero. If the strings are equal, zero is
 * returned. Else, a negative  integer is returned.
 */
int str_compare(const string_t *a, const string_t *b);

/*
 * str_contains returns true if the string str contains the substring substr at
 * any position.
 */
int str_contains(const string_t *str, const char *substr);

/*
 * str_contains_char returns true (>0) if string str contains the character c,
 * else returns false (0).
 */
int str_contains_char(const string_t *str, char c);

/*
 * str_prefixed returns true (>0) if string str begins with the string pref
 */
int str_prefixed(const string_t *str, const char *pref);

/*
 * str_suffixed returns true (>0) if string str ends with the string suff.
 */
int str_suffixed(const string_t *str, const char *suff);
