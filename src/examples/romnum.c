#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "romnum.h"

typedef struct numeral numeral_t;
struct numeral {
	unsigned short value;
	unsigned short max_follows;
	unsigned char max_consecutive;
	char subtracts_from;
};

static const numeral_t i = { 1, 0, 3, '\0' };
static const numeral_t v = { 5, 1, 1, 'i' };
static const numeral_t x = { 10, 9, 3, 'i' };
static const numeral_t l = { 50, 10, 1, 'x' };
static const numeral_t c = { 100, 90, 3, 'x' };
static const numeral_t d = { 500, 100, 1, 'c' };
static const numeral_t m = { 1000, 900, 3, 'c' };
static const numeral_t undef = { 0, 0, 0, '\0' };

static const numeral_t *lval(char ch)
{
	switch (ch) {
	case 'i': return &i;
	case 'v': return &v;
	case 'x': return &x;
	case 'l': return &l;
	case 'c': return &c;
	case 'd': return &d;
	case 'm': return &m;
	}
	return NULL;
}

int rntoi(const char *input)
{
	int acc = 0, value = 0, count = 0, i;
	char ch, prev_ch = '\0';
	numeral_t prev_numeral = undef;

	i = strlen(input);
	if (i == 0)
		return -1;

	for (i = i-1; i >= 0; --i, acc += value) {
		const numeral_t *numeral;

		ch = tolower(input[i]);
		if (ch == prev_ch) {
			if (++count > prev_numeral.max_consecutive)
				return -1;
			continue;
		}

		numeral = lval(ch);
		if (numeral == NULL) {
			return -1;
		}

		if (i > 0 && input[i-1] == numeral->subtracts_from) {
			/* This numeral is preceded by the numeral that can be
			 * subtracted away from this one. */
			value = numeral->value - lval(input[i-1])->value;
			count = 1;
			prev_ch = '\0';
			prev_numeral = undef;
			i -= 1;
		} else if (value <= numeral->max_follows) {
			/* The next numeral is going smaller, so this is OK */
			value = numeral->value;
			count = 1;
			prev_ch = ch;
			prev_numeral = *numeral;
		} else {
			return -1;
		}
	}

	return acc;
}

