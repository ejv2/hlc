/*
 * utf.h - C99 implementation of unicode parsing
 * Copyright (C) Ethan Marshall - 2023
 *
 * Requirements: stddef.h, stdlib.h, wchar.h
 *
 * Important note: This is *not* a UTF-8 implementation! It is a wrapper around
 * the standard C wchar routines for ease of use in simple cases. For pure
 * UTF-8 support, please use a more suitable library, such as libgrapheme.
 *
 * Additionally, you *must* have called setlocale(LC_ALL, "") at least once,
 * else these routines will decode UTF-16 by default on any platform (probably
 * not what you want).
 */

#ifdef HLC_AUTO_INCLUDE
#define UTF_AUTO_INCLUDE
#endif

#ifdef UTF_AUTO_INCLUDE
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#endif

/*
 * rune_t is a simple type alias for the system's defined wchar_t type.
 * This is mainly to differentiate results from this header and from standard
 * C. It is always safe to cast between rune_t and wchar_t without any data
 * loss. The types are identical.
 */
typedef wchar_t rune_t;

/*
 * utf_decode decodes a unicode encoded wide character string into a
 * heap-allocated buffer of runes and returns a pointer to the first rune. If
 * len is not null, the number of wide characters decoded is written to len,
 * which will also be the buffer size on the heap.
 */
static rune_t *utf_decode(const char *dec, size_t *len)
{
	size_t slen, ind = 0, clen = 16;
	rune_t *buf = malloc(sizeof(rune_t) * clen);

	/*
	 * calculate string length
	 * TODO: find a way to not require this
	 */
	for (const char *walk = dec; *walk; walk++, slen++);

	while (*dec) {
		int step = mbtowc(buf + ind, dec, slen);
		if (step < 0) {
			free(buf);
			return NULL;
		}

		size_t alen = (size_t)step;
		dec += alen, slen -= alen;
		ind++;
		if (ind == (clen - 1)) {
			clen *= 2;
			buf = realloc(buf, sizeof(rune_t) * clen);
			if (!buf)
				return NULL;
		}
	}

	if (len)
		*len = ind;

	return buf;
}

/*
 * utf_next decodes the next wide character from strp (at least *len
 * in size), incrementing the pointer to point at the next expected
 * byte and decrementing len to show the consumed bytes. This
 * function modifies its arguments. If you require a reference to the
 * string buffer or initial length, please copy the original pointer
 * first.
 */
static rune_t utf_next(char **strp, size_t *len)
{
	rune_t r = 0;
	size_t slen = mbtowc(&r, *strp, *len);
	*strp += slen;
	*len -= slen;

	return r;
}
