#include <stdio.h>
#include <stdlib.h>
#include<omp.h> 

#include "bucket_sort.h"

int main (void){
  int size=64,rangeB=8, nBuckets=8;
  float a[size];
  for (int i=0;i<size;i++){
      double d;
      d = drand48();
      d *= d;
      d *= nBuckets*rangeB;
      a[i]=(float)d;
  }

  //for(int i=0; i<14; i++)
    //printf("%f\n", a[i]);
  bucketSort(a, size,8,8);

  //imprimir a
  for(int i=0; i<size; i++)
    printf("%f\n", a[i]);
}

