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

int main(int argc, char **argv)
{
	vect_new(int, ivect);
	vect_append(&ivect, 1234);
	printf("%d\n", vect_get(&ivect, 0));
}

/* vim: ft=c
 */
