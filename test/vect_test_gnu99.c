/*
 * test cases for vect.h explicitly written to only compile on GNU C99, *not*
 * ISO C99.
 *
 * To compile and run this test, you *must* use the following CFLAGS:
 * 	-fnested-functions
 * 	-std=gnu99 OR -std=gnu11
 */

#ifndef __GNUC__
#error Requires compilation using gcc -std=gnu99 or later
#endif

#include <stdio.h>

#define HLC_AUTO_INCLUDE
#include "../vect.h"

int foreach(size_t i, int elem)
{
	printf("[%lu] => %d\n", i, elem);
}

int main(int argc, char **argv)
{
	vect_new(int, ivect);
	vect_append(&ivect, 1234);
	vect_append(&ivect, 5678);
	vect_foreach(&ivect, foreach);
	vect_destroy(&ivect);
}

/* vim: ft=c
 */
