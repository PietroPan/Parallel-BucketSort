# Parallel-BucketSort

Parallel Bucket Sorting [algorithm](https://en.wikipedia.org/wiki/Bucket_sort) with OpenMP

Get performance measuremnts with [Papi](https://icl.utk.edu/papi/)

## Run

Main takes 5 arguments:

Vector size (Default=1M)
Number of buckets (Default=256)
Number of runs (Default=10)
Array distribution, 0 for Uniform or 1 for Logarithmic (Default=0)

Set OMP_NUM_THREADS environment variable to get parallelization
