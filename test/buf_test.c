#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../buf.h"

static const char longish[] = "abcdefghijklmnopqrstuvwxyz";
static const size_t longlen = sizeof(longish)/sizeof(longish[0]);

typedef struct {
	char *strbuf;
	size_t len, cap;
} testbuf_t;

void test_attach()
{
	testbuf_t testbuf = (testbuf_t){
		.strbuf = malloc(sizeof(char) * 2),
		.len = 0,
		.cap = 2,
	};

	BUF_ATTACH(str, char, testbuf.strbuf, testbuf.len, testbuf.cap);
	for (int i = 0; i < 4; i++) {
		BUF_PUSH(str, 'a');
	}
	BUF_PUSH(str, '\0');
	BUF_DETACH(str, testbuf.strbuf, testbuf.len, testbuf.cap);

	printf("%s (%lu:%lu)\n", testbuf.strbuf, testbuf.len, testbuf.cap);
	if (strcmp(testbuf.strbuf, "aaaa") != 0) {
		fprintf(stderr, "string in detached buffer does not match\n");
		exit(1);
	}
	if (testbuf.len != sizeof("aaaa")) {
		fprintf(stderr, "wrong length in detached buffer (expect %lu)\n", sizeof("aaaa"));
		exit(1);
	}
	if (testbuf.cap < sizeof("aaaa")) {
		fprintf(stderr, "invalid buffer capacity, expect at least %lu", sizeof("aaaa"));
		exit(1);
	}

	free(testbuf.strbuf);
}

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

	test_attach();
}
