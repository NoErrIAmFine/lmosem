#include <div64.h>

unsigned int  __div64_32(unsigned long long *n, unsigned int base)
{
	unsigned long long rem = *n;
	unsigned long long b = base;
	unsigned long long res, d = 1;
	unsigned int high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (unsigned long long ) high << 32;
		rem -= (unsigned long long ) (high*base) << 32;
	}

	while ((signed long long )b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}