#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include "bucket_sort_par.h"
#include "mergeSort.h"
#include "quick_sort.h"
#include "math.h"

struct bucket {
  int total;
  float* value;
};

void bucketSortPar(float a[], int size, int nBuckets, int rangeB){
  //inicialização dos buckets
  struct bucket bucketsVector[nBuckets];
  int i,pos;
  
  int sB[nBuckets];
  for(i=0; i<nBuckets;i++) sB[i]=0;
  for(i=0; i<size; i++){
      int pos=(a[i]/rangeB*nBuckets);
      if (pos<0) pos=0;
      if (pos>=nBuckets) pos=nBuckets-1;
      sB[pos]++;
  }

  for(i=0; i<nBuckets; i++){
    bucketsVector[i].value = (float*)malloc(sizeof(float)*sB[i]);
    bucketsVector[i].total=0;
  }

  //colocar cada nr dentro de um bucket
  for(i=0; i<size; i++){
      pos=floor(a[i]/rangeB*nBuckets);
      if (pos<0) pos=0;
      if (pos>=nBuckets) pos=nBuckets-1;        
            bucketsVector[pos].value[bucketsVector[pos].total]=a[i];
            bucketsVector[pos].total++;  
  }

  int size_bucket, aux=0,j;

  //Ordenar cada bucket  (Podemos mudar o algoritmo para Radiz MSD, supostamente é o melhor para paralelizar)
  //Por os numeros ordenados no array inicial
  #pragma omp parallel for schedule(dynamic)
  for(i=0; i<nBuckets; i++){
    //id = omp_get_thread_num();
    //printf("Thread %d sorting bucket %d\n",id,i);
    //quickSort(bucketsVector[i].value, 0, bucketsVector[i].total-1);
    //mergeSort(bucketsVector[i].value, 0, bucketsVector[i].total-1);
  }

  for(i=0;i<nBuckets;i++){
    size_bucket=bucketsVector[i].total;
    //printf("[%.2f%% in bucket %d]\n",(float)size_bucket/(float)size*100,i);
    for(j=0; j<size_bucket; j++){
      a[aux+j]=bucketsVector[i].value[j];
    }
    aux+=size_bucket;
    free(bucketsVector[i].value);
  }

}

