/*
 * slice.h - C99 implementation of a slice buffer
 * Copyright (C) Ethan Marshall - 2023
 *
 * Requirements: stdlib.h stdio.h stdint.h string.h
 */

#ifdef HLC_AUTO_INCLUDE
#define SLC_AUTO_INCLUDE
#endif

#ifdef SLC_AUTO_INCLUDE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#endif

/*
 * slc_initial_cap is the initial capacity created through a call to slc_new.
 */
static const size_t slc_initial_cap = 2;

/*
 * slice_t is a container with a known length and capacity which may be
 * appended with a known quantity of data, re-sliced to expose a known
 * subsection of data and safely indexed. The slice guarantees that all data
 * within its capacity is ready to use and in a sane default state (zero
 * initialized). Reslicing a slice below its length does not discard trimmed
 * data - it is still available if resliced back to that length. A slice may
 * never be sliced negative and may not be sliced to above its capacity.
 *
 * An empty slice takes up one byte and has a length of zero. A
 * zero-initialized slice is ready to use and is identical to one with a zero
 * length.
 *
 * All instances of a slice share a copy of one underlying array. Slices
 * created from reslicing will have a flag set which prevents flag_free from
 * freeing the underlying array, eliminating some double free errors. The
 * downside of this design means that a slice may *never* be safely appended or
 * have its capacity otherwise changed after having been resliced at least
 * once. No sanity check is provided to guard against this. It is *up to you*
 * to ensure this does not occur.
 */
typedef struct {
	void *buf;
	/* elem size */
	size_t esize;
	/* not multiplied by elem size */
	size_t len, cap;
	/* subslice? */
	int8_t sub;
} slice_t;

static slice_t _slc_make(size_t size, size_t ilen, size_t icap)
{
	slice_t s;
	size_t cap;

	if (icap)
		cap = icap;
	else
		cap = slc_initial_cap;

	if (ilen > cap) {
		fprintf(stderr, "PANIC: invalid slice len/cap (len: %lu, cap: %lu)\n", ilen, cap);
		abort();
	}

	s.buf = calloc(cap, size);
	if (!s.buf) {
		fprintf(stderr, "PANIC: out of memory (slice alloc)\n");
		abort();
	}
	s.esize = size;
	s.len = ilen;
	s.cap = cap;
	s.sub = 0;

	return s;
}

/*
 * slc_make returns a new slice of length ilen and initial capacity icap, with
 * each element of type typename. If icap is zero, slc_initial_cap is used
 * instead.
 */
#define slc_make(typename, len, cap) _slc_make(sizeof(typename), len, cap)

static slice_t _slc_new(size_t size)
{
	return _slc_make(size, 0, 0);
}

/*
 * slc_new returns a new slice of length zero and initial capacity of
 * slc_initial_cap, with each element of type typename.
 */
#define slc_new(typename) _slc_new(sizeof(typename))

/*
 * slc_free frees all memory associated with a slice, unless said slice was
 * created via subslicing another slice, in which case this call is a no-op.
 *
 * After calling slc_free on the base slice, all subslices are invalidated and
 * are to be treated as dangling references.
 */
static void slc_free(slice_t *s)
{
	if (s->sub)
		return;

	free(s->buf);
	s->len = s->cap = 0;
}

/*
 * slc_len returns the current length of the slice, in units of elements.
 */
static size_t slc_len(slice_t *s)
{
	return s->len;
}

/*
 * slc_cap returns the current capacity of the slice, in units of elements.
 * This is the maximum index which the slice may be resliced to.
 */
static size_t slc_cap(slice_t *s)
{
	return s->cap;
}

/*
 * slc_buflen returns the current length of the slice's buffer, in units of
 * bytes. This is the actual length of the underlying array.
 */
static size_t slc_buflen(slice_t *s)
{
	return s->esize * s->len;
}

/*
 * slc_bufcap returns the current capacity of the slice's buffer, in units of
 * bytes. This is not particularly useful in reslicing or indexing.
 */
static size_t slc_bufcap(slice_t *s)
{
	return s->esize * s->cap;
}

/*
 * slc_grow grows the slice s to have at least the capacity cap. If allocation
 * fails, slc_grow panics, printing debugging information to standard error. If
 * cap is less than the current capacity, no operation is performed.
 */
static void slc_grow(slice_t *s, size_t cap)
{
	int8_t *walk;
	void *alloc;

	if (s->cap >= cap)
		return;
	if (s->sub)
		return; /* TODO: need a proper solution for this */
	if (cap * s->esize < cap) {
		fprintf(stderr, "PANIC: slice exceeded maximum capacity (%lu elements, %lu bytes each)\n",
				cap, s->esize);
		abort();
	}

	alloc = realloc(s->buf, cap * s->esize);
	if (!alloc) {
		fprintf(stderr, "PANIC: out of memory (slice realloc)\n");
		abort();
	}
	s->buf = alloc;

	for (walk = (int8_t *)s->buf + s->cap; walk < (int8_t *)s->buf + cap; walk++) {
		*walk = 0;
	}
	s->cap = cap;
}

/*
 * slc_ref returns a slice which can safely be used for pass-by-reference. It
 * will not be freed upon calls to slc_free and will only cause dangling
 * references in the case of an erroneous slc_free or slc_grow on the base
 * slice.
 */
static slice_t slc_ref(slice_t *base)
{
	slice_t ret = *base;
	ret.sub = 1;

	return ret;
}

/*
 * slc_reslice returns a new slice which is a resliced version of the original.
 * It acts as a "window" (similar to std::stringview) into the buffer and acts
 * like an entirely new slice with the contents from the base slice copied. The
 * base slice must remain valid and the same capacity while any subslices are
 * in use. If the base slice changes capacity, all subslices are invalidated
 * references and must be regenerated.
 *
 * The range given is a half open range. The lower bound is inclusive, whereas
 * the upper bound is excluded. If a reslice is attempted past the slice's
 * capacity, or lower > upper, slc_reslice panics.
 */
static slice_t slc_reslice(slice_t *base, size_t lower, size_t upper)
{
	slice_t ret = slc_ref(base);

	if (lower >= slc_cap(base) || upper > slc_cap(base)) {
		fprintf(stderr, "PANIC: reslice bounds out of range (bounds [%lu:%lu] with cap %lu)\n",
				lower, upper, slc_cap(base));
		abort();
	}
	if (lower >= upper) {
		fprintf(stderr, "PANIC: invalid reslice bounds (bounds [%lu:%lu] with cap %lu)\n",
				lower, upper, slc_cap(base));
		abort();
	}

	ret.buf = (int8_t *)ret.buf + (lower * base->esize);
	ret.len = upper - lower;
	ret.cap -= lower;

	return ret;
}

/*
 * slc_copy copies the contents from src into dst, filling as many elements as
 * possible before slc_len(dst) is exhausted (not slc_cap(dst)!). This allows
 * for copying between subslices. Source and destination may overlap. The
 * number of elements copied are returned, which will be the minimum of the
 * length of the source and the length of the destination. If the size of the
 * elements in the source and destination are not the same, slc_copy panics.
 */
static size_t slc_copy(slice_t *dst, slice_t *src)
{
	size_t count;

	if (dst->esize != src->esize) {
		fprintf(stderr, "PANIC: destination and source slice type mismatch (src: %lu, dst: %lu)\n",
				src->esize, dst->esize);
		abort();
	}

	if (slc_len(dst) <= slc_len(src))
		count = slc_len(dst);
	else
		count = slc_len(src);

	memmove(dst->buf, src->buf, count * src->esize);
	return count;
}
