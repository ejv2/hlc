#include <locale.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>

#include "../utf.h"

int main(void)
{
	size_t l = 0;
	char *testptr = "lots of nice unicode: $££$\n";
	size_t len = strlen(testptr);

	setlocale(LC_ALL, "");

	rune_t *buf = utf_decode(testptr, &l);
	if (!buf) {
		perror("utf decode");
		abort();
	}

	char *walk = testptr;
	size_t wlen = len;
	for (size_t i = 0; i < l; i++) {
		rune_t r = utf_next(&walk, &wlen);
		wprintf(L"%lu => %lc | %lc\n", wlen, r, buf[i]);
		if (r != buf[i]) {
			wprintf(L"expected utf_decode and utf_next to agree, got different results (dec: %lu, next: %lu)\n", buf[i], r);
			return 1;
		}
	}
	free(buf);
}
