#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>

static int limit = 200000000; // Maximum is Integer.MAX_VALUE - 1;
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

struct thread_data_t {
	pthread_t t;
	int start;
	int n_primes;
};

struct thread_data_t threads[N_CORES];

void *worker ( void *arg) {
	struct thread_data_t *t = static_cast<struct thread_data_t *>(arg);
	int step = N_CORES * 2;
	int n_primes = 0;
	for (int c = t->start; c <= limit; c += step) {
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
	t->n_primes = n_primes;
	return NULL;
}

// ***********************************************************************
// Count all primes from 3 up to limit
// ***********************************************************************

int main(int ac, char **av) {
	printf("Counting primes from 3 to %d\n",limit);
	printf("Size of int is %ld bytes\n",sizeof(int));

	double start = now();
	for (int i=0; i<N_CORES; i++) {
		threads[i].start = 3+i*2;
		threads[i].n_primes = 0;
		pthread_create(&threads[i].t,NULL,worker,static_cast<void *>(&threads[i]));
	}
	int n_primes = 1;
	for (int i=0; i<N_CORES; i++) {
		pthread_join(threads[i].t,NULL);
		n_primes += threads[i].n_primes;
	}
	double stop = now();

	double exectime = stop-start;
	printf("%d primes found\n",n_primes);
	printf("Time to complete %f seconds\n",exectime); // in seconds
}
