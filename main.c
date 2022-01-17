#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "papi.h"

#include "bucket_sort.h"
#include "bucket_sort_par.h"

static int read_command_line (int argc, char *argv[], int *v_size, int *nBuckets, int *nRuns, int *info);
static int alloc_vector (int **v, int v_size);
static int ini_vector (int **v, int v_size);
static int free_vector (int **v);
static float my_rand (int N);
//static void getDistribution(int v[],int N,int nBuckets);

//void (*func)(float *, int);
#define NUM_EVENTS 4
int Events[NUM_EVENTS] = { PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L1_DCM, PAPI_L2_DCM};
// PAPI counters' values
long long values[NUM_EVENTS], min_values[NUM_EVENTS], total_values[NUM_EVENTS];
int retval, EventSet=PAPI_NULL;

int main (int argc, char *argv[]) {
    int v_size=1E8,nBuckets=1024,nRuns=10,info=1;//input
    long long start_usec, end_usec, elapsed_usec, min_usec=0L,total_usec=0L;
    int i, run,num_hwcntrs = 0,th=atoi(getenv("OMP_NUM_THREADS"));
    int *v;

    read_command_line(argc,argv,&v_size,&nBuckets,&nRuns,&info);

    printf ( "\nSetting up PAPI...");
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
    fprintf(stdout, "done!\nThis system has %d available counters.\n", num_hwcntrs);
    
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
    if (!ini_vector (&v, v_size)) return 0;
    //fprintf (stdout, "done!\n");

    // warmup caches
    fprintf (stdout, "Warming up caches...");
    bucketSort(v, v_size, nBuckets);
    free_vector(&v);
    if (!ini_vector (&v, v_size)) return 0;
    fprintf (stdout, "done!\n");

    fprintf (stdout, "\nVector Size: %d\nNumber of Bucktes: %d\n",v_size,nBuckets);
    if (th>1) fprintf (stdout, "Parallel Algorithm with %d threads\n",th);
    else fprintf (stdout, "Sequencial Algorithm\n");

    for (run=0 ; run < nRuns ; run++) {
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

        bucketSort(v, v_size, nBuckets);

        /* Stop counting events */
        if (PAPI_stop(EventSet,values) != PAPI_OK) {
            fprintf (stderr, "PAPI error stoping counters!\n");
            return 0;
        }
        
        end_usec = PAPI_get_real_usec();
        //fprintf (stdout, "done!\n");
        
        elapsed_usec = end_usec - start_usec;
        //fprintf (stdout,"\nSort:%d wall clock time: %lld usecs", run,elapsed_usec);
        total_usec+=elapsed_usec;
        for (i=0;i<NUM_EVENTS;i++) total_values[i] += values[i];
        if ((run==0) || (elapsed_usec < min_usec)) {
            min_usec = elapsed_usec;
            for (i=0 ; i< NUM_EVENTS ; i++) min_values[i] = values [i];
        }
        /*for (i=0;i<v_size;i++){
          fprintf(stdout,"%d\n",v[i]);
        }*/
        //printf("\n");
	    free_vector (&v);
        if (!ini_vector (&v, v_size)) return 0;
    } // end runs
    //getDistribution(v,v_size,nBuckets);

    //fprintf(stdout,"\nMin wall clock time: %lld usecs\n", min_usec);
    fprintf(stdout,"\nWall clock time: %lld usecs\n", total_usec/nRuns);
    // output PAPI counters' values
    for (i=0 ; i< NUM_EVENTS ; i++) {
        char EventCodeStr[PAPI_MAX_STR_LEN];
        
        if (PAPI_event_code_to_name(Events[i], EventCodeStr) == PAPI_OK && info>0) {
            fprintf (stdout, "%s = %lld\n", EventCodeStr, total_values[i]/nRuns);
        } else {
            fprintf (stdout, "PAPI UNKNOWN EVENT = %lld\n", min_values[i]);
        }
    }
    
    free_vector (&v);
    fprintf (stdout,"\nThat's all, folks\n");
    return 0;
}

int read_command_line (int argc, char *argv[], int *v_size, int *nBuckets, int *nRuns, int *info){

    if (argc>1) *v_size=atoi(argv[1]);
    if (argc>2) *nBuckets=atoi(argv[2]);
    if (argc>3) *nRuns=atoi(argv[3]);
    if (argc>4) *info=atoi(argv[4]);

    return 1;
}


void getDistribution(int v[],int N,int nBuckets){
    printf("\n");
    int dist[nBuckets];
    memset(dist,0,nBuckets*sizeof(int));
    int i,j,pos;
    for(i=0; i<N; i++){
        pos=floor((float)v[i]/N*nBuckets);
        if (pos<0) pos=0;
        if (pos>=nBuckets) pos=nBuckets-1;
        dist[pos]++;
     }

    float aux=(float)nBuckets/(float)16;
    int nD=floor(aux),mnD,cnt=0, distV;
    float decI=aux-nD,dec=decI;
    printf("%f\n",decI);
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

int alloc_vector (int **v, int N) {
    
    *v = (int *) malloc (N*sizeof(int));
    if (!(*v)) {
        printf("Could not allocate memory for vector!\n");
        return 0;
    }
    return 1;
}

float my_rand (int N) {
    double d;
    
    d = drand48 ();
    //d -=0.5;
    //d *= drand48();
    //d -=0.5;
    d *= N;
    return ((int)d);
}

int ini_vector (int **v, int N) {
    int i;
    int *ptr;
    
    if (!alloc_vector (v, N)) return 0;
    for (i=0 , ptr = (*v) ; i<N ; i++ , ptr++) {
        *ptr = my_rand(N);
    }
    return 1;
} 

int free_vector (int **v) {
    free (*v);
    *v = NULL;
    return 1;
}
