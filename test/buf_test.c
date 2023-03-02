#include <stdlib.h>
#include <stdio.h>

#include "../buf.h"

static const char longish[] = "abcdefghijklmnopqrstuvwxyz";
static const size_t longlen = sizeof(longish)/sizeof(longish[0]);

int main()
{
	BUF_NEW(char, a);
	
	printf("%lu:%lu\n", BUF_LEN(a), BUF_CAP(a));
	for (size_t i = 0; i < longlen; i++) {
		BUF_PUSH(a, longish[i]);
		printf("%lu:%lu\n", BUF_LEN(a), BUF_CAP(a));
	}
	printf("%s\n", a);
	printf("0: %p (%c); 26: %p (%c); 27: %p (NULL)\n", BUF_GET(a, 0), *BUF_GET(a, 0),
			BUF_GET(a, 26), *BUF_GET(a, 26), BUF_GET(a, 27));
	if (BUF_GET(a, 27)) {
		fprintf(stderr, "27th element should return NULL");
		exit(1);
	}

	BUF_FREE(a);
}
