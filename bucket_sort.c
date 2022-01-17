#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include "mergeSort.h"
#include "bucket_sort.h"
#include "quick_sort.h"
struct bucket {
  int total;
  float* value;
};


void bucketSort(float a[], int size, int nBuckets, int rangeB){
  //inicialização dos buckets
  struct bucket bucketsVector[nBuckets];

  int i, size_r=size, nBuckets_r=nBuckets, rangeB_r=rangeB;
  float* r = a;
  
  for(i=0; i<nBuckets_r; i++){
    bucketsVector[i].value = (float*)malloc(sizeof(float)*size_r);
    bucketsVector[i].total=0;
  }

  //colocar cada nr dentro de um bucket
  int pos;
  for(i=0; i<size_r; i++){
      pos = (r[i]/rangeB_r*nBuckets_r);
      if (pos<0) pos=0;
      if (pos>=nBuckets_r) pos=nBuckets_r-1;
      bucketsVector[pos].value[bucketsVector[pos].total]=a[i];
      bucketsVector[pos].total++;
   }

  int size_bucket, aux,j;

  //Ordenar cada bucket  (Podemos mudar o algoritmo para Radiz MSD, supostamente é o melhor para paralelizar)
  //Por os numeros ordenados no array inicial
  
  #pragma omp parallel for schedule(dynamic)
  for(i=0;i<nBuckets_r;i++)
    quickSort(bucketsVector[i].value, 0, bucketsVector[i].total-1);
  
  for(i=0,aux=0; i<nBuckets_r; i++){
    size_bucket=bucketsVector[i].total;
    //printf("[%.2f%% in bucket %d]\n",(float)size_bucket/(float)size*100,i);
    for(j=0; j<size_bucket; j++){
      r[aux+j]=bucketsVector[i].value[j];
    }
    aux+=size_bucket;
    free(bucketsVector[i].value);
  }

}