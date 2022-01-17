#include <stdio.h>
#include "quick_sort.h"

// function to swap elements
void swap(float *a, float *b) {
  float t = *a;
  *a = *b;
  *b = t;
}

// function to find the partition position
int partition(float array[], int low, int high) {
  float pivot = array[high];
  int i = (low - 1),j;

  for (j = low; j < high; j++) {
    if (array[j] <= pivot) {
      i++;
      swap(&array[i], &array[j]);
    }
  }
  swap(&array[i + 1], &array[high]);

  return (i + 1);
}

void quickSort(float array[], int low, int high) {
  if (low < high) {
    int pi = partition(array, low, high);
    quickSort(array, low, pi - 1);
    quickSort(array, pi + 1, high);
  }
}
