/*
 * str.c - C99 implementation of a high-level string
 * Copyright (C) Ethan Marshall - 2023
 *
 * NOTE: This is a source-header library. You must both compile this source
 * file and include the corresponding header.
 *
 * Requirements: corresponding str.h be in include path
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#include "str.h"

/* Initial string buffer size allocated by str_new in bytes */
#define STR_INITIAL_BUFSIZ 32

/* The maximum size of a string in bytes */
#define STR_SIZE_MAX ((size_t)-1)

string_t str_new()
{
	char *buf = calloc(STR_INITIAL_BUFSIZ, sizeof(char));
	return (string_t){
		.s = buf,
		.e = buf,
		.cap = STR_INITIAL_BUFSIZ,
	};
}

void str_free(string_t *str)
{
	free(str->s);
	str->e = str->s = NULL;
	str->cap = 0;
}

size_t str_cap(const string_t *str)
{
	return (str->cap == 0) ? 0 : str->cap - 1;
}

size_t str_len(const string_t *str)
{
	/* need to subtract one if non-zero, as the null byte does not count */
	return str->e - str->s;
}

int str_grow(string_t *str, size_t delta)
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

int str_reserve(string_t *str, size_t delta)
{
	size_t cap = str_cap(str);
	size_t len = str_len(str);

	if (cap - len >= delta) {
		return -1;
	}

	return str_grow(str, delta);
}

void str_compact(string_t *str)
{
	size_t len = str->e - str->s;
	char *buf = realloc(str->s, len + 1);
	if (!buf)
		return;

	str->cap = len + 1;
	str->s = buf;
	str->e = str->s + len;
}

void str_truncate(string_t *str, size_t len)
{
	if (len >= (size_t)(str->e - str->s)) {
		return;
	}

	str->e = str->s + len;
	*str->e = '\0';
}

void str_reset(string_t *str)
{
	str_truncate(str, 0);
}

string_t str_from(const char *cstr)
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

char *str_cstr(const string_t *str)
{
	return str->s;
}

string_t str_clone(const string_t *str)
{
	if (str_len(str) == 0) {
		return (string_t){NULL, NULL, 0};
	}
	return str_from(str_cstr(str));
}

string_t str_concat(const string_t *a, const string_t *b)
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

char str_get(string_t *s, size_t i)
{
	if (i > str_len(s)) {
		printf("PANIC: string index out of range (i: %lu, len: %lu)\n", i, str_len(s));
		abort();
	}

	return s->s[i];
}

void str_set(string_t *s, size_t i, char c)
{
	if (i > str_len(s)) {
		printf("PANIC: string index out of range (i: %lu, len: %lu)\n", i, str_len(s));
		abort();
	}

	s->s[i] = c;
}

void str_append(string_t *dst, const string_t *src)
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

string_t str_fmt(const char *fmt, ...)
{
	string_t str = (string_t){NULL, NULL, 0};
	int wlen = 0, written = 0;
	va_list tmp, args;

	va_start(args, fmt);
	/* need a copy so that vsprintf does not mess everything up with va_arg */
	va_copy(tmp, args);

	wlen = vsnprintf(NULL, 0, fmt, args);
	if (wlen < 0)
		return str;
	if (!str_grow(&str, wlen + 1))
		return str;
	va_end(args);

	written = vsprintf(str.s, fmt, tmp);
	va_end(tmp);

	if (written < 0) {
		str_free(&str);
		return (string_t){NULL, NULL, 0};
	}
	str.e = str.s + written;

	return str;
}

int str_equal(const string_t *a, const string_t *b)
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

int str_compare(const string_t *a, const string_t *b)
{
	const char *wa = a->s, *wb = b->s;
	for (; *wa==*wb && *wa; wa++, wb++);
	return *(unsigned char *)wa - *(unsigned char *)wb;
}

int str_contains(const string_t *str, const char *substr)
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

int str_contains_char(const string_t *str, char c)
{
	for (const char *walk = str->s; walk != str->e; walk++) {
		if (*walk == c)
			return 1;
	}

	return 0;
}

int str_prefixed(const string_t *str, const char *pref)
{
	for (const char *walk = str->s, *swalk = pref; *swalk; walk++, swalk++) {
		if (*walk != *swalk)
			return 0;
	}

	return 1;
}

int str_suffixed(const string_t *str, const char *suff)
{
	const char *walk = str->e, *swalk;
	size_t sufflen = 0;

	/* first we need strlen of suffix */
	for (const char *walk = suff; *walk; walk++, sufflen++);

	/* walk back to where the suffix should begin */
	walk -= sufflen;
	if (walk < str->s)
		return 0;

	for (swalk = suff; walk < str->e; walk++, swalk++) {
		if (*walk != *swalk)
			return 0;
	}

	return 1;
}
