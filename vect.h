/*
 * vect.h - C99/C11 implementation of a type-safe vector
 * Copyright (C) Ethan Marshall - 2023
 *
 * Requirements: stdlib.h stdio.h stdint.h string.h
 */

#ifdef HLC_AUTO_INCLUDE
#define VECT_AUTO_INCLUDE
#endif

#ifdef VECT_AUTO_INCLUDE
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#endif

/*
 * _vect_t is the internal generic vector implementation. It is not for
 * external use, except in exceedingly rare circumstances, which will be clear
 * to you if and when they arise.
 *
 * This implementation is *truly* generic, in that it uses a void * etc. It is
 * safe to use after being zero initialized via memset. Although void * is used
 * to pass without type safety, the vector copies elements by value, such that
 * a bad pointer (or one which goes out of scope) cannot cause a bad access.
 */
struct _vect_t {
	void *buf;
	size_t len, cap;
};

/*
 * Internal: resize v such that its capacity is exactly new. If new is zero
 * _vect_resize will double the existing capacity. If new is less than the
 * existing capacity, no operation is performed. If allocation fails, zero is
 * returned, else one.
 */
static int _vect_resize(struct _vect_t *v, size_t tsiz, size_t new)
{
	size_t newcap = new;
	void *newbuf;

	if (new == 0)
		newcap = (v->cap + 1) * 2;
	if (newcap <= v->cap)
		return 1;

	newbuf = realloc(v->buf, tsiz * newcap);
	if (!newbuf)
		return 0;
	v->buf = newbuf;
	v->cap = newcap;

	return 1;
}

/*
 * Internal: append t (of size ts) to v by value. A copy will be made and ts
 * bytes will be memcpy'd from t. If resizing fails for any reason, no
 * operation is performed and zero is returned.
 */
static int _vect_append(struct _vect_t *v, void *t, size_t ts)
{
	uintptr_t arith;
	if ((v->len + 1) >= v->cap) {
		if (!_vect_resize(v, ts, 0))
			return 0;
	}
	v->len++;

	arith = (uintptr_t)v->buf + ((v->len - 1) * ts);
	memcpy((void *)arith, t, ts);
	return 1;
}

/*
 * Internal: returns a pointer to the value at index ind, or NULL if ind is out
 * of range.
 */
static void *_vect_get(struct _vect_t *v, size_t ind, size_t ts)
{
	if (ind >= v->len) {
		return NULL;
	}

	return (void *)((uintptr_t)v->buf + (ind * ts));
}

/*
 * Internal: sets the element at ind to the value val (of size ts).
 *
 * TODO: This has some inherent overhead over direct assignment, as we don't
 * know the type, so we can't just use the assgnment operator as we normally
 * would. Instead, we use a memcpy, which is probably worse (maybe?).
 */
static int _vect_set(struct _vect_t *v, size_t ind, size_t ts, void *val)
{
	uintptr_t arith;
	if (ind >= v->len)
		return 0;

	arith = (uintptr_t)v->buf + (ind * ts);
	memcpy((void *)arith, val, ts);
	return 1;
}

/*
 * Internal: truncates the vector down to zero elements with no reallocations
 * and no change to capacity.
 */
static void _vect_clear(struct _vect_t *v)
{
	v->len = 0;
}

/*
 * Internal: checks if an element is present in the vector which is identical
 * to the passed value (checked using memcmp).
 */
static int _vect_contains(struct _vect_t *v, size_t ts, void *val)
{
	for (size_t i = 0; i < v->len; i++) {
		if (memcmp(val, (void *)((uintptr_t)v->buf + i), ts) == 0) {
			return 1;
		}
	}

	return 0;
}

/*
 * vect_typename returns an autogenerated typename for a generic vector for the
 * given type.
 */
#define vect_typename(type) vect_##type

/*
 * Internal use only: Used to set custom storage to make nested function decls
 * work out OK. All functions are declared with storage class tstore (default
 * static).
 */
#define _vect_declare(type, tname, tstore)						\
	typedef int(*tname##_iterfunc)(size_t i, type elem);				\
	typedef struct tname##_struct {							\
		struct _vect_t v;							\
		/* function impls */							\
		void (*append)(struct tname##_struct *this, type t);			\
		type (*get)(struct tname##_struct *this, size_t ind);			\
		void (*set)(struct tname##_struct *this, size_t ind, type val);		\
		size_t (*len)(struct tname##_struct *this);				\
		size_t (*cap)(struct tname##_struct *this);				\
		void (*foreach)(struct tname##_struct *this, tname##_iterfunc iter);	\
		int (*empty)(struct tname##_struct *this);				\
		int (*contains)(struct tname##_struct *this, type val);			\
		void (*clear)(struct tname##_struct *this);				\
		void (*destroy)(struct tname##_struct *this);				\
	} tname;									\
	tstore void tname##_vect_append(struct tname##_struct *this, type t) {		\
		if (!_vect_append(&this->v, &t, sizeof(type))) {			\
			fprintf(stderr, "PANIC: out of memory (vector alloc)\n");	\
			abort();							\
		}									\
	}										\
	tstore void tname##_vect_destroy(struct tname##_struct *this) {			\
		free(this->v.buf);							\
		this->v.cap = this->v.len = 0;						\
	}										\
	tstore type tname##_vect_get(struct tname##_struct *this, size_t ind) { 			\
		type *t = (type *)_vect_get(&this->v, ind, sizeof(type));				\
		if (!t) {										\
			fprintf(stderr, "PANIC: vector access out of range (index: %lu, len: %lu)\n",	\
					ind, this->v.len);						\
			abort();									\
		}											\
		return *t;										\
	}												\
	tstore void tname##_vect_set(struct tname##_struct *this, size_t ind, type val)			\
	{												\
		if (!_vect_set(&this->v, ind, sizeof(type), &val)) {					\
			fprintf(stderr, "PANIC: vector set out of range (index: %lu, len: %lu)\n",	\
					ind, this->v.len);						\
			abort();									\
		}											\
	}												\
	tstore void tname##_vect_foreach(struct tname##_struct *this, tname##_iterfunc iter)		\
	{												\
		for (size_t i = 0; i < this->v.len; i++) {						\
			if (!iter(i, *((const type *)_vect_get(&this->v, i, sizeof(type)))))		\
				return;									\
		}											\
	}												\
	tstore size_t tname##_vect_len(struct tname##_struct *this)	\
	{								\
		return this->v.len;					\
	}								\
	tstore size_t tname##_vect_cap(struct tname##_struct *this)	\
	{								\
		return this->v.cap;					\
	}								\
	tstore int tname##_vect_empty(struct tname##_struct *this)	\
	{								\
		return this->v.len == 0;				\
	}								\
	tstore void tname##_vect_clear(struct tname##_struct *this)	\
	{								\
		_vect_clear(&this->v);					\
	}								\
	tstore int tname##_vect_contains(struct tname##_struct *this, type val)	\
	{									\
		return _vect_contains(&this->v, sizeof(val), &val);		\
	}									\
	tstore tname tname##_vect_init() {		\
		tname ret;				\
		ret.v.buf = NULL;			\
		ret.v.len = 0;				\
		ret.v.cap = 0;				\
		ret.append = tname##_vect_append;	\
		ret.get = tname##_vect_get;		\
		ret.set = tname##_vect_set;		\
		ret.len = tname##_vect_len;		\
		ret.cap = tname##_vect_cap;		\
		ret.foreach = tname##_vect_foreach;	\
		ret.empty = tname##_vect_empty;		\
		ret.contains = tname##_vect_contains;	\
		ret.destroy = tname##_vect_destroy;	\
		ret.clear = tname##_vect_clear;		\
		return ret;				\
	}						\
	/* little hack to allow portable		\
	 * semi colons after vect_declares */		\
	struct _vect_decl_isoc_workaround

/*
 * vect_declare declares a new vector of the given type. It is type-safe for
 * that type and will not accept elements not of this type. It is mutated with
 * using function pointer calls inside of the provided struct, which are
 * auto-generated at compile time.
 *
 * Note: If you are using GCC, you can conveniently declare this within another
 * function and have the nested function definitions work correctly.
 */
#define vect_declare(type, tname) _vect_declare(type, tname, static)

/*
 * vect_new declares a new vector of the given type and a new variable
 * with the name vname. This is mainly for convenience for temporary variables.
 *
 * Note: This will not work correctly unless your compiler supports nested
 * function declarations. If you are looking to zero-initialize the structure
 * correctly, simply use the return value of vect_init.
 */
#define vect_new(type, vname) 					\
	_vect_declare(type, type##vname, inline);		\
	type##vname vname = type##vname##_vect_init();		\
	/* see above for info about this little hack */		\
	struct _vect_decl_isoc_workaround

/*
 * vect_init returns an initialized vector. You must have already declared your
 * vector for the type you wish to use under the type you pass to this
 * function.
 *
 * If you have used vect_new for the vector, you need not call vect_init (not
 * that you could anyway, as vect_new's typenames are deliberately scrambled).
 */
#define vect_init(tname) tname##_vect_init()

/*
 * vect_destroy frees all storage associated with the vector vect. It is
 * possible to re-use a vector after having destroyed it, but its storage will
 * have been reset and all contents lost.
 */
#define vect_destroy(vect) (vect)->destroy(vect)

/*
 * vect_get returns the element at index ind from the vector vect. If ind is
 * out of range, vect_get calls abort with a panic message.
 */
#define vect_get(vect, ind) ((vect)->get(vect, ind))

/*
 * vect_set sets the element at index ind to val. If ind is out of range,
 * vect_set calls abort with a panic message.
 */
#define vect_set(vect, ind, val) (vect)->set(vect, ind, val)

/*
 * vect_len returns the number of elements stored in vect.
 */
#define vect_len(vect) ((vect)->len(vect))

/*
 * vect_cap returns the number of elements which can be appended to vect
 * without a reallocation.
 */
#define vect_cap(vect) ((vect)->cap(vect))

/*
 * vect_empty returns true (>0) if vect contains no elements, else false (0).
 */
#define vect_empty(vect) ((vect)->empty(vect))

/*
 * vect_contains returns true (>0) if vect contains the given element (checked
 * for equality using memcmp), else false (0).
 */
#define vect_contains(vect, val) ((vect)->contains(vect, val))

/*
 * vect_clear truncates the vector to zero elements, but does not free any
 * memory or reallocate. The capacity remains unchanged.
 *
 * To free memory, see vect_destroy.
 */
#define vect_clear(vect) (vect)->clear(vect)

/*
 * vect_append appends val to the end of the vector pointed to by vect.
 */
#define vect_append(vect, val) (vect)->append(vect, val)

/*
 * vect_foreach calls the handle function iter for each element contained in
 * vect. iter must accept a size_t argument i and a <type> argument elem, which
 * will be the index and item (passed by value) respectively.
 */
#define vect_foreach(vect, iter) (vect)->foreach(vect, iter)
