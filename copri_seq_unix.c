#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

// ***********************************************************************
// Measuring time
// ***********************************************************************

double timestamp ( clockid_t clk_id ) {
	struct timespec t;
	int ret = clock_gettime(clk_id,&t);
	assert(ret == 0);
	double ts = ((double) t.tv_sec) + ((double) t.tv_nsec) / 10e9;
	return ts; // In seconds
}

double now () {
	return timestamp(CLOCK_REALTIME);
}

double clock_resolution_ns ( clockid_t clk_id ) {
	struct timespec t;
	int ret = clock_getres(clk_id,&t);
	assert(ret == 0);
	assert(t.tv_sec == 0);
	return (double) t.tv_nsec;
}

// ***********************************************************************
// Count all primes from 3 up to limit
// ***********************************************************************

static int n_primes = 1;

int main(int ac, char **av) {
	if (sizeof(int) != 4) {
		fprintf(stderr,"sizeof(int) is not 4 (%lu instead)\n",sizeof(int));
		exit(-1);
	}
	if (ac != 2) {
		fprintf(stderr,"usage: %s <limit> - count primes in [1,limit); limit==-1: MAX_INT-1\n",av[0]);
		exit(-1);
	}
	int limit = atoi(av[1]);
	if (limit == -1)
		limit = INT_MAX-1;

	double start = now();
	for (int c=3; c<=limit; c+=2) {
		bool a_prime = true;
		for (int d=3; d*d<=c; d+=2) {
			if (c % d == 0) {
				a_prime = false;
				break;
			}
		}
		if (a_prime) n_primes++;
	}
	double stop = now();

	double exectime = stop-start;

	printf("%s, limit=%d, n_primes=%d, time(s)=<%e>\n",av[0],limit,n_primes,exectime);
}
