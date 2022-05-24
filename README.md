# __copri__ - Count Primes

This code project contains various approaches to count the number of primes in a given intervall [1,limit) with a focus on different strategies for parallelization. The inner loop to check whether a candidate is a prime number or not is always the same - it is not intended to optimize that loop.

List of approaches:
- `copri_seq`: sequential solution running on a single core without any core affinity.
- `copri_pthreads_atomic`: a given number of pthreads test primes in parallel. The next candidate number and the total count of primes are accessed by all these threads. To avoid race conditions, this variables are atomic ints.
- `copri_pthreads_isolated`: a given number of pthreads test independent stripes of candidates, thus no synchronization is needed.
- `copri_cuda`: A CUDA parallel solution to count the number of primes. Requires a nVidia graphics card.

## Structure

The Python script `run_experiments.py` coordinates the execution of a set of experiments with different limits and - if applicable - with different number of threads.

The folder `results` stores CSV files for any successful series of experiments. These files can be analyzed and compared using the jupyter notebook `copri_analyze.ipynb`. The last part of any csv file identifies the computer used for the experiemnt. The file `cpus.txt` should contain all the technical details that are needed to compare any results.

## TODO

- `run_experiments.py`: specify the number of threads as a command line parameter; specify a comment string as a command line parameter which will be the header of the csv file (CPU description)
- `copri_pthreads_isolated` still not accepting command line arguments
- A makefile is needed to compile all the executables