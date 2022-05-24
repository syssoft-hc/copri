#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

#define CHECK(call)															\
{																			\
	const cudaError_t error = call;											\
	if (error != cudaSuccess) {												\
		printf("Error: %s:%d ",__FILE__,__LINE__);							\
		printf("code: %d, reason: %s\n",error,cudaGetErrorString(error));	\
		exit(1);															\
	}																		\
}

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
// CUDA kernel to test candidate c for prime number
// To decease possible congestion while incrementing the number of
// primes found, an array of counters indexed by the threadIdx.x
// is used. Still, threads with identical id but in different blocks
// might run in parallel if the GPU has multiple processors (28 in
// my case), so an atomic add operation is needed.
// ***********************************************************************

__global__ void prime_test( int *counter ) {
	int c = (blockIdx.x * blockDim.x + threadIdx.x) * 2 + 3;
	bool a_prime = true;
	for (int d = 3; d*d <= c; d += 2) {
		if (c % d == 0) {
			a_prime = false;
			break;
		}
	}
	if (a_prime) {
		atomicAdd(&(counter[threadIdx.x]), 1);
	}
}

// ***********************************************************************
// Count all primes from 3 up to limit using the streaming capabilities
// of an CUDA-capable GPU
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
	int limit = atoi(av[1]);
	if (limit == -1)
		limit = INT_MAX-1;


	CHECK(cudaSetDevice(0));
	cudaDeviceProp deviceProp;
	CHECK(cudaGetDeviceProperties(&deviceProp, 0));
	// printf("CUDA device warp size is %d\n", deviceProp.warpSize);
	// printf("CUDA device max threads per block is %d\n", deviceProp.maxThreadsPerBlock);
	// printf("CUDA device max grid size is %d\n", deviceProp.maxGridSize[0]);

	int block_size = 1000;
	assert(block_size < deviceProp.maxThreadsPerBlock);
	int grid_size = limit / block_size / 2;
	assert(grid_size < deviceProp.maxGridSize[0]);

	// printf("block size is %d and grid size (number of blocks) is %d\n", block_size, grid_size);

	double start = now();

	int *counter = (int *) malloc(block_size * sizeof(int));
	for (int i = 0; i < block_size; i++)
		counter[i] = 0;

	int *dev_counter = 0;
	CHECK(cudaMalloc((void**) &dev_counter, block_size * sizeof(int)));
	CHECK(cudaMemcpy(dev_counter, counter, block_size * sizeof(int), cudaMemcpyHostToDevice));

	prime_test <<<grid_size,block_size>>> (dev_counter);
	CHECK(cudaDeviceSynchronize());

	int n_primes = 1;
	CHECK(cudaMemcpy(counter, dev_counter, block_size * sizeof(int), cudaMemcpyDeviceToHost));
	for (int i = 0; i < block_size; i++)
		n_primes += counter[i];

	double stop = now();
	double exectime = stop - start;

	printf("%s, limit=%d, n_primes=%d, time(s)=<%e>\n",av[0],limit,n_primes,exectime);

	cudaFree(dev_counter);
	free(counter);

	cudaDeviceReset();
}
