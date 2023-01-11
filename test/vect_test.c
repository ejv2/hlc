/* This is not yet a proper testing script and merely shows debugging output
 * TODO: Proper testing program/script
 */

#include <stdio.h>

#define HLC_AUTO_INCLUDE
#include "../vect.h"

vect_declare(int, vector_int);
vect_declare(const char *, vector_str);

int main(void)
{
	vector_int v = vect_init(vector_int);
	vect_append(&v, 1234);
	printf("%d\n", vect_get(&v, 0));
	printf("aftermath: (len: %lu, cap: %lu)\n", vect_len(&v), vect_cap(&v));

	vect_append(&v, 5678);
	printf("%d\n", vect_get(&v, 1));
	printf("aftermath: (len: %lu, cap: %lu)\n", vect_len(&v), vect_cap(&v));

	vect_append(&v, 0x100);
	printf("%d\n", vect_get(&v, 2));
	printf("aftermath: (len: %lu, cap: %lu)\n", vect_len(&v), vect_cap(&v));
	vect_set(&v, 2, 0x101);
	printf("%d\n", vect_get(&v, 2));
	printf("aftermath: (len: %lu, cap: %lu)\n", vect_len(&v), vect_cap(&v));

	printf("contains value? %d\n", vect_contains(&v, 0x101));

	vector_int v2 = vect_init(vector_int);
	printf("empty? %s\n", vect_empty(&v) ? "true" : "false");
	printf("empty (v2)? %s\n", vect_empty(&v2) ? "true" : "false");

	vect_clear(&v);
	printf("empty (v1 aftermath)? %s\n", vect_empty(&v) ? "true" : "false");
	printf("aftermath details: (len: %lu, cap: %lu)\n", vect_len(&v), vect_cap(&v));

	const char *check = "Ethan";
	vector_str v3 = vect_init(vector_str);
	vect_append(&v3, check);
	printf("%s\n", vect_get(&v3, 0));

	vect_destroy(&v);
	vect_destroy(&v2);
	vect_destroy(&v3);
}
