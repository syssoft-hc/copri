#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

#include <atomic>

using namespace std;

static int limit;
static const int N_CORES = 16;

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
// worker code executed by a thread
// ***********************************************************************

static atomic<int> candidate(3);
static atomic<int> n_primes(1);

void *worker ( void *arg) {
	while (true) {
		int c = candidate.fetch_add(2, std::memory_order_relaxed);
		if (c >= limit) return NULL;
		bool a_prime = true;
		for (int d=3; d*d<=c; d+=2) {
			if (c % d == 0) {
				a_prime = false;
				break;
			}
		}
		if (a_prime) {
			n_primes++;
		}
	}
}

// ***********************************************************************
// Count all primes from 3 up to limit
// ***********************************************************************

int main(int ac, char **av) {
	if (sizeof(int) != 4) {
		fprintf(stderr,"sizeof(int) is not 4 (%lu instead)\n",sizeof(int));
		exit(-1);
	}
	if (ac != 2) {
		fprintf(stderr,"usage: %s <limit> - count primes in [1,limit); limit==-1: MAX_INT-1\n",av[0]);
		exit(-1);
	}
	limit = atoi(av[1]);
	if (limit == -1)
		limit = INT_MAX-1;

	double start = now();
	pthread_t threads[N_CORES];
	for (int i=0; i<N_CORES; i++) {
		int rval = pthread_create(&threads[i],NULL,worker,NULL);
		if (rval != 0) {
			fprintf(stderr,"pthread_create fails with error=%d\n",errno);
			exit(-1);
		}
	}
	for (int i=0; i<N_CORES; i++)
		pthread_join(threads[i],NULL);
	double stop = now();

	double exectime = stop-start;

	printf("%s, limit=%d, n_primes=%d, time(s)=<%e>\n",av[0],limit,n_primes.load(),exectime);
}
