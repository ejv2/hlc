#include <stdio.h>
#include <stdlib.h>

#define SLC_AUTO_INCLUDE
#include "../slice.h"

void test_new()
{
	slice_t s = slc_new(int16_t);
	printf("[%lu:%lu] [%lu:%lu]\n", slc_len(&s), slc_cap(&s), slc_buflen(&s), slc_bufcap(&s));

	slice_t s1 = slc_make(int16_t, 5, 10);
	printf("[%lu:%lu] [%lu:%lu]\n", slc_len(&s1), slc_cap(&s1), slc_buflen(&s1), slc_bufcap(&s1));

	slc_free(&s);
	slc_free(&s1);
}

void test_grow()
{
	size_t walk, oldcap;
	slice_t s = slc_new(int8_t);

	/* test invariance when too small */
	oldcap = slc_cap(&s);
	slc_grow(&s, 0);
	if (slc_cap(&s) != oldcap) {
		printf("bad realloc: requested cap=0 (too small), cap=%lu\n", slc_cap(&s));
		exit(1);
	}

	slc_grow(&s, oldcap + 10);
	if (slc_cap(&s) != oldcap + 10) {
		printf("bad realloc: requested cap+=10, cap=%lu\n", slc_cap(&s));
		exit(1);
	}
	for (walk = 0; walk < s.cap; walk++) {
		if (((int8_t *)s.buf)[walk] != 0) {
			printf("buffer was not zeroed at index %lu\n", walk);
			exit(1);
		}
	}

	if (slc_len(&s) != 0) {
		printf("len changed: slice realloc changed length");
		exit(1);
	}

	slc_free(&s);
}

void test_ref()
{
	slice_t a = slc_new(int8_t);
	slice_t ref = slc_ref(&a);

	slc_free(&a);
	slc_free(&ref);
}

void test_reslice()
{
	int8_t *walk;

	slice_t a = slc_make(int16_t, 0, 4);
	/* should fill buffer as [0 0 1 1] */
	for (walk = (int8_t *)a.buf; walk < (int8_t *)a.buf + slc_bufcap(&a); walk++) {
		*walk = (walk - (int8_t *)a.buf) / 3;
	}

	size_t i;
	slice_t lower = slc_reslice(&a, 0, 2);
	slice_t upper = slc_reslice(&a, 2, slc_cap(&a));

	if (slc_len(&lower) != 2) {
		printf("wrong lower slice length (len: %lu)\n", slc_len(&lower));
		exit(1);
	}
	if (slc_cap(&lower) != 4) {
		printf("wrong lower slice cap (cap: %lu)\n", slc_cap(&lower));
		exit(1);
	}

	if (slc_len(&upper) != 2) {
		printf("wrong upper slice length (len: %lu)\n", slc_len(&upper));
		exit(1);
	}
	if (slc_cap(&upper) != 2) {
		printf("wrong upper slice cap (cap: %lu)\n", slc_cap(&upper));
		exit(1);
	}

	for (i = 0; i < slc_len(&lower); i++) {
		int8_t val = ((int8_t *)lower.buf)[i];
		printf("lower[%ld] = %d\n", i, val);
		if (val != 0) {
			printf("^^^ wrong value for lower ^^^");
			exit(1);
		}
	}
	for (i = 0; i < slc_len(&upper); i++) {
		int8_t val = ((int8_t *)upper.buf)[i];
		printf("upper[%ld] = %d\n", i, val);
		if (val != 1) {
			printf("^^^ wrong value for upper ^^^");
			exit(1);
		}
	}

	slc_free(&a);
	slc_free(&lower);
	slc_free(&upper);
}

int main()
{
	test_new();
	test_grow();
	test_ref();
	test_reslice();
}
