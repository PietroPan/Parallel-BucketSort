#!/bin/bash

#SBATCH --output=cores_%j.log     # Standard output and error log

MY=/home/pg46950/Parallel-BucketSort/
VECTOR_SIZE=1000000
N_BUCKETS=256
N_RUNS=1
PRINT_INFO=1


module load papi/5.4.1

export OMP_NUM_THREADS=32
$MY/sort  $VECTOR_SIZE $N_BUCKETS $PARALLEL $N_RUNS $PRINT_INFO