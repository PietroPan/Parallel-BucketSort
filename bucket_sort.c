#include <stdio.h>
#include <stdlib.h>
#include "bucket_sort.h"

struct bucket {
  int total;
  float* value;
};

//se quisermos radix sort MSD https://www.geeksforgeeks.org/msd-most-significant-digit-radix-sort/
/////////////////////MERGESORT/////////////////////////

void merge(float arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    /* create temp arrays */
    float L[n1], R[n2];

    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    /* Copy the remaining elements of L[], if there
    are any */
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    /* Copy the remaining elements of R[], if there
    are any */
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

/* l is for left index and r is right index of the
sub-array of arr to be sorted */
void mergeSort(float arr[], int l, int r)
{
    if (l < r) {
        // Same as (l+r)/2, but avoids overflow for
        // large l and h
        int m = l + (r - l) / 2;

        // Sort first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);

        merge(arr, l, m, r);
    }
}


///////////////////////////////////////////////////////



void bucketSort(float a[], int size){
  //inicialização dos buckets
  struct bucket bucketsVector[10];
  int i;
  for(i=0; i<10; i++){
    bucketsVector[i].value = (float*)malloc(sizeof(float)*size);
    bucketsVector[i].total=0;
  }

  //colocar cada nr dentro de um bucket
  int pos;
  for(i=0; i<size; i++){
      pos = a[i]/10;
      bucketsVector[pos].value[bucketsVector[pos].total]=a[i];
      bucketsVector[pos].total++;
   }

  int size_bucket, aux,j;

  //Ordenar cada bucket  (Podemos mudar o algoritmo para Radiz MSD, supostamente é o melhor para paralelizar)
  //Por os numeros ordenados no array inicial
  for(i=0,aux=0; i<10; i++){
    mergeSort(bucketsVector[i].value, 0, bucketsVector[i].total-1);
    size_bucket=bucketsVector[i].total;
    for(j=0; j<size_bucket; j++){
      a[aux+j]=bucketsVector[i].value[j];
    }
    aux+=size_bucket;
    free(bucketsVector[i].value);
  }

}
