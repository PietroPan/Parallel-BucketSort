#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include "mergeSort.h"
#include "bucket_sort.h"

struct bucket {
  int total;
  float* value;
};


void bucketSort(float a[], int size, int nBuckets, int rangeB){
  //inicialização dos buckets
  struct bucket bucketsVector[nBuckets];
  int i;
  for(i=0; i<nBuckets; i++){
    bucketsVector[i].value = (float*)malloc(sizeof(float)*size);
    bucketsVector[i].total=0;
  }

  //colocar cada nr dentro de um bucket
  int pos;
  for(i=0; i<size; i++){
      pos = a[i]/rangeB;
      if (pos<0) pos=0;
      if (pos>=nBuckets) pos=nBuckets-1;
      bucketsVector[pos].value[bucketsVector[pos].total]=a[i];
      bucketsVector[pos].total++;
   }

  int size_bucket, aux,j;

  //Ordenar cada bucket  (Podemos mudar o algoritmo para Radiz MSD, supostamente é o melhor para paralelizar)
  //Por os numeros ordenados no array inicial
  for(i=0,aux=0; i<nBuckets; i++){
    mergeSort(bucketsVector[i].value, 0, bucketsVector[i].total-1);
    size_bucket=bucketsVector[i].total;
    //printf("[%.2f%% in bucket %d]\n",(float)size_bucket/(float)size*100,i);
    for(j=0; j<size_bucket; j++){
      a[aux+j]=bucketsVector[i].value[j];
    }
    aux+=size_bucket;
    free(bucketsVector[i].value);
  }

}
