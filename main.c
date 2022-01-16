#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include "bucket_sort.h"
#include "bucket_sort_par.h"
#include <time.h>
#include <string.h>
#include <math.h>

void getDistribution2(float v[],int N,int nBuckets,int rangeB){
    printf("\n");
    int dist[nBuckets];
    memset(dist,0,nBuckets*sizeof(int));
    int i,j,pos;
    for(i=0; i<N; i++){
        pos = floor(v[i]/rangeB*nBuckets);
        //printf("%d\n",pos);
        if (pos<0) pos=0;
        if (pos>=nBuckets) pos=nBuckets-1;
        dist[pos]++;
     }

    printf("Bucket Distribution:\n");
    float aux=(float)nBuckets/(float)16;
    int nD=floor(aux),mnD,cnt=0, distV;
    float decI=aux-nD,dec=decI;
    for(i=0; i<16; i++){
        if(dec>=1) {
            mnD=nD+1;
            dec-=1;
        }
	else mnD=nD;
        for(j=0,distV=0;j<mnD;j++,cnt++){
            distV+=dist[cnt];
        }
	if (mnD==1) printf("[%.2f%% in bucket %d]\n",(float)distV/(float)N*100,cnt-1);
        else if (mnD>1) printf("[%.2f%% in buckets %d..%d]\n",(float)distV/(float)N*100,cnt-mnD,cnt-1);
        dec+=decI;
    }
}



int main (void){
  int size=1E8,rangeB=1, nBuckets=1024,N=5;
  double before,after;
  double elapsed,total;
  for (int i=0;i<N;i++){
    float* a=(float*)malloc(sizeof(float)*size);
    for (int i=0;i<size;i++){
        double d;
        d = drand48();
        //d *= d;
        d *= rangeB;
        a[i]=(float)d;
    }

    before = omp_get_wtime();
    bucketSortPar(a, size,nBuckets,rangeB);
    after = omp_get_wtime();

        elapsed = (after-before);
        printf("Sort[%d] took %lf sec\n",i,elapsed);
        total += elapsed;
    //imprimir a
    //for(int i=0; i<size; i++)
      //  printf("%f\n", a[i]);

    //getDistribution2(a,size,nBuckets,rangeB);
    free(a);
  }
  printf("Mean wall clock time: %lf sec\n",total/N);

}
