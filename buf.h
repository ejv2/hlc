/*
 * buf.h - utility macros for temporary buffers
 * Copyright (C) Ethan Marshall - 2023
 *
 * Requirements: stdlib.h
 */

#ifdef HLC_AUTO_INCLUDE
#define BUF_AUTO_INCLUDE
#endif

#ifdef BUF_AUTO_INCLUDE
#include <stdlib.h>
#endif

/*
 * LENGTH returns how many elements are in a static data structure. The size of
 * x must be known at compile time and must never be a raw pointer.
 */
#ifndef LENGTH
#define LENGTH(x) (sizeof(x)/sizeof(x[0]))
#endif

/*
 * BUF_NEW creates a new heap buffer of elements type typename and buffer
 * variable name bufname. The buffer initially starts with two element capacity
 * and doubles on reallocation.
 */
#define BUF_NEW(typename, bufname)				\
	typename *bufname = malloc(sizeof(typename) * 2);	\
	size_t bufname##_len = 0, bufname##_cap = 2;

/*
 * BUF_ATTACH inherits buffer data from already stored information. All normal
 * buffer routines may then be used with bufname as the name. Information is
 * stored back to the origin only after a call to BUF_DETACH
 */
#define BUF_ATTACH(bufname, typename, buf, len, cap)		\
	typename *bufname = buf;				\
	size_t bufname##_len = len, bufname##_cap = cap;	\

/*
 * BUF_DETACH stores metadata about the buffer called bufname back into its
 * original position. Every call to BUF_ATTACH must be matched with at least
 * one call to BUF_DETACH. If BUF_FREE is called on a local buffer, you must
 * still detach it.
 *
 * Note: The buffer remains valid after a call to BUF_DETACH, but you must call
 * it again if you modify your copy of the buffer.
 */
#define BUF_DETACH(bufname, buf, len, cap)	\
	buf = bufname;				\
	len = bufname##_len;			\
	cap = bufname##_cap;			\

/*
 * BUF_LEN returns the number of elements in the buffer.
 */
#define BUF_LEN(bufname) (bufname##_len)

/*
 * BUF_CAP returns the number of elements which can fit in the buffer with no
 * reallocation.
 */
#define BUF_CAP(bufname) (bufname##_cap)

/*
 * BUF_PUSH appends a new item into the buffer. If memory reallocation occurs
 * and fails, the call blocks until memory is available. No checks are done
 * against overflows of the size counters, as you will be *way* over your
 * system memory limit by then.
 */
#define BUF_PUSH(bufname, elem)					\
	if (bufname##_len == bufname##_cap) {			\
		void *tmp = NULL;				\
		while (!tmp)					\
			tmp = realloc(bufname,			\
					sizeof(*bufname) * 	\
					bufname##_cap * 2);	\
		bufname = tmp;					\
		bufname##_cap *= 2;				\
	}							\
	bufname[bufname##_len] = elem;				\
	bufname##_len++;					\

/*
 * BUF_GET returns a pointer to the element at the ith index, or null if it is
 * out of range.
 */
#define BUF_GET(bufname, i) ((i >= bufname##_len) ? NULL : bufname + i)

/*
 * BUF_FREE frees all memory associated with the given heap buffer and sets all
 * counters associated with it to zero.
 */
#define BUF_FREE(bufname) free(bufname); \
	bufname##_len = 0; \
	bufname##_cap = 0;
