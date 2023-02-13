#include <stdio.h>
#include <stdlib.h>

#define UTYPE_AUTO_INCLUDE
#include "../utypes.h"

int main()
{
	int a = 42;

	intptr aip = (intptr)&a;
	uintptr aup = (uintptr)&a;

	printf("orig: %p, intptr: %p, uintptr: %p\n", (void *)&a, (void *)aip, (void *)aup);
	if (*(int *)aip != 42 || *(int *)aup != 42) {
		printf("cast to intptr or uintptr did not conserve bit pattern\n");
		exit(1);
	}
}
