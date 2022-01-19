#!/bin/bash

#SBATCH --output=logs/slurm-%j.log # Standard output and error log
module load papi/5.4.1

MY=/home/pg46950/Parallel-BucketSort/
VECTOR_SIZE=1000000
N_BUCKETS=256
N_RUNS=20
ARRAY_DIST=0 #0=Normal 1=Logarithmic
export OMP_NUM_THREADS=40

$MY/sort  $VECTOR_SIZE $N_BUCKETS $N_RUNS $ARRAY_DIST