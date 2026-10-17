#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../stack.h"

struct complex_type {
	int a;
	char b[16];
} ct;

int main()
{
	size_t cap = 2;
	struct complex_type *buf = malloc(cap * sizeof(struct complex_type));
	stack_t stk = stk_make(buf, cap);

	ct.a = 1;
	int a = stk_push(&stk, &ct);

	ct.a = 2;
	int b = stk_push(&stk, &ct);

	ct.a = 3;
	int c = stk_push(&stk, &ct);

	/* expect a&b to succeed but c to fail */
	if (!(a && b && !c)) {
		printf("FAIL: push to full stack accepted/push to non-full stack failed (got %d%d%d, expect 110)\n", a, b, c);
		exit(1);
	}


	struct complex_type ctc;
	for (int i = 2; i > 0; i--) {
		stk_pop(&stk, &ctc);
		printf("popped %d\n", ctc.a);
		if (ctc.a != i) {
			printf("FAIL: pushed value != popped value (expect %d, got %d)\n", i, ctc.a);
			exit(1);
		}
	}

	if (stk_pop(&stk, &ctc)) {
		printf("FAIL: pop accepted on empty stack (base: %p, top %p)\n", stk.base, stk.top);
		exit(1);
	}

	a = stk_push(&stk, &ct);
	b = stk_push(&stk, &ct);
	if (!a || !b) {
		printf("FAIL: valid stack push rejected after emptying [using pop] (base: %p, top %p)\n", stk.base, stk.top);
		exit(1);
	}

	stk_empty(&stk);
	if (stk_len(&stk) != 0) {
		printf("FAIL: stack length nonzero after empty (base: %p, top %p, len %lu)\n", stk.base, stk.top, stk_len(&stk));
		exit(1);
	}

	a = stk_push(&stk, &ct);
	b = stk_push(&stk, &ct);
	if (!a || !b) {
		printf("FAIL: valid stack push rejected after emptying [using empty] (base: %p, top %p)\n", stk.base, stk.top);
		exit(1);
	}

	stk_resize(&stk, buf, cap);
	if (stk_len(&stk) != 2) {
		printf("FAIL: stack length changed after resize [no change] (base: %p, top %p, expect length: 2, got: %lu)\n", stk.base, stk.top, stk_len(&stk));
		exit(1);
	}

	cap++;
	buf = realloc(buf, cap * sizeof(*buf));
	stk_resize(&stk, buf, cap);
	if (stk_len(&stk) != 2) {
		printf("FAIL: stack length changed after resize [growing] (base: %p, top %p, expect length: 2, got: %lu)\n", stk.base, stk.top, stk_len(&stk));
		exit(1);
	}
	if (!stk_push(&stk, &ct)) {
		printf("FAIL: valid push rejected after growing (base: %p, top %p, len: %lu)\n", stk.base, stk.top, stk_len(&stk));
		exit(1);
	}

	cap = 1;
	stk_resize(&stk, buf, cap);
	if (stk_len(&stk) != 1) {
		printf("FAIL: stack length incorrect after resize [shrinking] (base: %p, top %p, expect length: 1, got: %lu)\n", stk.base, stk.top, stk_len(&stk));
		exit(1);
	}
	if (stk_push(&stk, &ct)) {
		printf("FAIL: full stack accepted push after shrinking (base: %p, top %p)\n", stk.base, stk.top);
		exit(1);
	}

	stk_empty(&stk);
	ct.a = 0;
	stk_push(&stk, &ct);

	struct complex_type *cptr = ((struct complex_type *)stk_index(&stk, 0));
	if (cptr->a != 0) {
		printf("FAIL: incorrect element returned by index (expect a=0, got a=%d)\n", cptr->a);
		exit(1);
	}

	free(buf);
}
