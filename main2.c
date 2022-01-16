#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "papi.h"

#include "bucket_sort.h"
#include "bucket_sort_par.h"

static int alloc_vector (float **v, int v_size);
static int ini_vector (float **v, int v_size, int rangeB);
static int free_vector (float **v);
static float my_rand (int rangeB);
static void getDistribution(float v[],int N,int nBuckets,int rangeB);

//void (*func)(float *, int);
#define NUM_EVENTS 4
int Events[NUM_EVENTS] = { PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L1_DCM, PAPI_L2_DCM};
// PAPI counters' values
long long values[NUM_EVENTS], min_values[NUM_EVENTS];
int retval, EventSet=PAPI_NULL;

#define NUM_RUNS 50

int main (int argc, char *argv[]) {
    double before,after,elapsed,total=0;
    long long start_usec, end_usec, elapsed_usec, min_usec=0L,mean_usec=0L;
    int v_size=1E7,nBuckets=1024,rangeB=1, i, run;
    float *v;
    int num_hwcntrs = 0;

    fprintf (stdout, "\nSetting up PAPI...");
    // Initialize PAPI
    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr,"PAPI library init error!\n");
        return 0;
    }
    
    /* create event set */
    if (PAPI_create_eventset(&EventSet) != PAPI_OK) {
        fprintf(stderr,"PAPI create event set error\n");
        return 0;
    }
    
    
    /* Get the number of hardware counters available */
    if ((num_hwcntrs = PAPI_num_hwctrs()) <= PAPI_OK)  {
        fprintf (stderr, "PAPI error getting number of available hardware counters!\n");
        return 0;
    }
    fprintf(stdout, "done!\nThis system has %d available counters.\n\n", num_hwcntrs);
    
    // We will be using at most NUM_EVENTS counters
    if (num_hwcntrs >= NUM_EVENTS) {
        num_hwcntrs = NUM_EVENTS;
    } else {
        fprintf (stderr, "Error: there aren't enough counters to monitor %d events!\n", NUM_EVENTS);
        return 0;
    }
    
    if (PAPI_add_events(EventSet,Events,NUM_EVENTS) != PAPI_OK)  {
        fprintf(stderr,"PAPI library add events error!\n");
        return 0;
    }

    // ini Vector
    //fprintf (stdout, "Initializing vector A...");
    if (!ini_vector (&v, v_size,rangeB)) return 0;
    //fprintf (stdout, "done!\n");

    // warmup caches
    fprintf (stdout, "Warming up caches...");
    bucketSortPar(v, v_size, nBuckets, rangeB);
    free_vector(&v);
    if (!ini_vector (&v, v_size,rangeB)) return 0;
    fprintf (stdout, "done!\n");

    for (run=0 ; run < NUM_RUNS ; run++) {
        //fprintf (stdout, "run=%d - Sorting Vector...\n", run);
        // use PAPI timer (usecs) - note that this is wall clock time
        // for process time running in user mode -> PAPI_get_virt_usec()
        // real and virtual clock cycles can also be read using the equivalent
        // PAPI_get[real|virt]_cyc()
        start_usec = PAPI_get_real_usec();
        
        /* Start counting events */
        if (PAPI_start(EventSet) != PAPI_OK) {
            fprintf (stderr, "PAPI error starting counters!\n");
            return 0;
        }
        before=omp_get_wtime();
        bucketSortPar(v, v_size, nBuckets, rangeB);
        after=omp_get_wtime();
        elapsed=(after-before);
        total += elapsed;

        /* Stop counting events */
        if (PAPI_stop(EventSet,values) != PAPI_OK) {
            fprintf (stderr, "PAPI error stoping counters!\n");
            return 0;
        }
        
        end_usec = PAPI_get_real_usec();
        //fprintf (stdout, "done!\n");
        
        elapsed_usec = end_usec - start_usec;
        //fprintf (stdout,"\nSort:%d wall clock time: %lld usecs", run,elapsed_usec);
        mean_usec+=elapsed_usec;
        
        if ((run==0) || (elapsed_usec < min_usec)) {
            min_usec = elapsed_usec;
            for (i=0 ; i< NUM_EVENTS ; i++) min_values[i] = values [i];
        }
        /*for (i=0;i<v_size;i++){
          fprintf(stdout,"%f\n",v[i]);
        }*/
        //printf("\n");
	    free_vector (&v);
        if (!ini_vector (&v, v_size,rangeB)) return 0;
    } // end runs
    getDistribution(v,v_size,nBuckets,rangeB);

    fprintf (stdout,"\nMin wall clock time: %lld usecs\n", min_usec);
    fprintf (stdout,"Mean wall clock time: %lld usecs\n", mean_usec/NUM_RUNS);
    fprintf(stdout,"Mean wall clock time: %lf sec\n\n",total/NUM_RUNS);
    // output PAPI counters' values
    for (i=0 ; i< NUM_EVENTS ; i++) {
        char EventCodeStr[PAPI_MAX_STR_LEN];
        
        if (PAPI_event_code_to_name(Events[i], EventCodeStr) == PAPI_OK) {
            fprintf (stdout, "%s = %lld\n", EventCodeStr, min_values[i]);
        } else {
            fprintf (stdout, "PAPI UNKNOWN EVENT = %lld\n", min_values[i]);
        }
    }
    
    free_vector (&v);
    fprintf (stdout,"\nThat's all, folks\n");
    return 0;
}

void getDistribution(float v[],int N,int nBuckets,int rangeB){
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

int alloc_vector (float **v, int N) {
    
    *v = (float *) malloc (N*sizeof(float));
    if (!(*v)) {
        printf("Could not allocate memory for vector!\n");
        return 0;
    }
    return 1;
}

float my_rand (int rangeB) {
    double d;
    
    d = drand48 ();
    //d -=0.5;
    //d *= drand48();
    //d -=0.5;
    d *= rangeB;
    return ((float)d);
}

int ini_vector (float **v, int N,int rangeB) {
    int i;
    float *ptr;
    
    if (!alloc_vector (v, N)) return 0;
    for (i=0 , ptr = (*v) ; i<N ; i++ , ptr++) {
        *ptr = my_rand(rangeB);
    }
    return 1;
} 

int free_vector (float **v) {
    free (*v);
    *v = NULL;
    return 1;
}
