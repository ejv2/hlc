/*
 * stack.h - C99 implementation of a (possibly variable sized) stack
 * Copyright (C) Ethan Marshall - 2023
 *
 * Requirements: stdio.h stdlib.h string.h
 */

#ifdef HLC_AUTO_INCLUDE
#define STK_AUTO_INCLUDE
#endif

#ifdef STK_AUTO_INCLUDE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

/*
 * stack_t is a container attached to an existing block of memory that will be
 * managed as a FILO stack for alike objects. stack_t does not contain any
 * elements in of itself, instead storing what is more akin to a stack frame.
 *
 * Objects of the same size can be added (pushed) or removed (popped) from the
 * top of the stack. Objects at any index can be accessed but cannot be removed.
 * Where it is possible, runtime type checking is performed to ensure only the
 * correct type is pushed.
 */
typedef struct {
	/* base pointer */
	void *base;
	/* pointer to top of stack */
	void *top;
	/* elem size */
	size_t esize;
	/* underlying buffer capacity */
	size_t cap;
} stack_t;

static stack_t _stk_make(void *base, size_t esize, size_t cap)
{
	return (stack_t){base, base, esize, cap};
}

static int _stk_push(stack_t *stk, void *elem, size_t esize)
{
	if (esize != stk->esize) {
		fprintf(stderr, "PANIC: push to stack of wrong type (stack type size %lu, push size %lu)\n", stk->esize, esize);
		abort();
	}
	if (!elem) {
		fprintf(stderr, "PANIC: NULL elem passed to stk_push (bp: %p, hp: %p, len: %lu)\n", stk->base, stk->top, (char *)stk->top - (char *)stk->base);
		abort();
	}

	/* stack full */
	if ((unsigned)((char *)stk->top - (char *)stk->base) >= stk->cap * stk->esize) {
		return 0;
	}

	memcpy(stk->top, elem, esize);
	stk->top = (void *)((char *)stk->top + stk->esize);
	return 1;
}

/*
 * stk_len returns the number of elements currently pushed onto the stack
 */
static size_t stk_len(stack_t *stk)
{
	return ((char *)stk->top - (char *)stk->base) / stk->esize;
}

/*
 * stk_pop removes the element at the top of the stack, writing its value to
 * dest, which must be a buffer of at least the element size.
 *
 * If the stack is empty, returns zero. Else, returns nonzero.
 *
 * If dest is NULL, no write occurs but the element is still popped.
 */
static int stk_pop(stack_t *stk, void *dest)
{
	if (stk_len(stk) == 0)
		return 0;

	if (dest)
		memcpy(dest, (char *)stk->top - stk->esize, stk->esize);

	stk->top = (void *)((char *)stk->top - stk->esize);
	return 1;
}

/*
 * stk_index returns a pointer to the element at zero-indexed offset ind from
 * the stack base. The pointer remains valid until the stack is resized or that
 * index is popped, and can be used to modify the contained data.
 *
 * If ind is greater than or equal to the stack length, stk_index panics.
 */
static void *stk_index(stack_t *stk, size_t ind)
{
	if (ind >= stk_len(stk)) {
		fprintf(stderr, "PANIC: stack index out of range (attempted index %lu, stack length %lu)\n", ind, stk_len(stk));
		abort();
	}

	return (void *)((char *)stk->base + ind * stk->esize);
}

/*
 * stk_empty truncates the stack to zero length
 */
static void stk_empty(stack_t *stk)
{
	stk->top = stk->base;
}

static void _stk_resize(stack_t *stk, void *newbuf, size_t newcap, size_t esize)
{
	if (esize != stk->esize) {
		fprintf(stderr, "PANIC: stack reallocated to wrong buffer type (stack type size %lu, push size %lu)\n", stk->esize, esize);
		abort();
	}

	size_t oldlen = stk_len(stk);
	stk->base = newbuf;
	stk->cap = newcap;
	stk->top = (void *)((char *)stk->base + oldlen * stk->esize);

	/* some elements discarded - truncate to full capacity */
	if (oldlen > newcap) {
		stk->top = (void *)((char *)stk->base + newcap * stk->esize);
	}
}

/*
 * stk_make returns a stack_t set up to manage buf as an empty stack.
 * The size of contained elements is detected from the pointer type. Void
 * pointer stacks are not supported. The stack can grow to at most cap elements
 * (not bytes).
 */
#define stk_make(buf, cap) _stk_make(buf, sizeof(*(buf)), cap)

/*
 * stk_push copies *elem to the top of the stack.
 * Returns zero if the stack is full, nonzero otherwise.
 *
 * If elem is NULL, stk_push panics.
 * If elem is not the same type as the buffer stores, stk_push panics.
 */
#define stk_push(stk, elem) _stk_push(stk, elem, sizeof(*elem))

/*
 * stk_resize grows/shrinks the stack's capacity to support at most newcap
 * elements. The caller is responsible for reallocating the underlying buffer
 * (for instance through realloc) to physically support this space.
 *
 * If the new buffer's element size is not the same as the old, stk_resize
 * panics.
 * If the new buffer's capacity is less than the previous, the elements over
 * capacity are treated as discarded and the contracted stack will be full.
 */
#define stk_resize(stk, newbuf, newcap) _stk_resize(stk, newbuf, newcap, sizeof(*newbuf))
