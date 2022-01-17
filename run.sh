#!/bin/bash

#SBATCH --output=cores_%j.log     # Standard output and error log

MY=/home/pg46950/TP/
VECTOR_SIZE=1000000
N_BUCKETS=124
N_RUNS=50
MAX_NUMBER=1
PRINT_INFO=1


module load papi/5.4.1

export OMP_NUM_THREADS=1
$MY/sort  $VECTOR_SIZE $N_BUCKETS $PARALLEL $N_RUNS $MAX_NUMBER $PRINT_INFO