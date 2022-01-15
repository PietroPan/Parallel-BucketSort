#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "papi.h"

#include "bucket_sort.h"

static int alloc_vector (float **v, int v_size);
static int ini_vector (float **v, int v_size);
static int free_vector (float **v);
static float my_rand (void);

//void (*func)(float *, int);
#define NUM_EVENTS 4
int Events[NUM_EVENTS] = { PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L1_DCM, PAPI_L2_DCM};
// PAPI counters' values
long long values[NUM_EVENTS], min_values[NUM_EVENTS];
int retval, EventSet=PAPI_NULL;

#define NUM_RUNS 5

int main (int argc, char *argv[]) {
    long long start_usec, end_usec, elapsed_usec, min_usec=0L;
    int v_size=1024, i, run;
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
    fprintf (stdout, "Initializing vector A...");
    if (!ini_vector (&v, v_size)) return 0;
    fprintf (stdout, "done!\n");

    // warmup caches
    fprintf (stdout, "Warming up caches...");
    bucketSort (v, v_size);
    fprintf (stdout, "done!\n");

    for (run=0 ; run < NUM_RUNS ; run++) {
        fprintf (stdout, "run=%d - Sorting Vector...", run);
        
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
        
        bucketSort(v, v_size);
        /* Stop counting events */
        if (PAPI_stop(EventSet,values) != PAPI_OK) {
            fprintf (stderr, "PAPI error stoping counters!\n");
            return 0;
        }
        
        end_usec = PAPI_get_real_usec();
        fprintf (stdout, "done!\n");
        
        elapsed_usec = end_usec - start_usec;
        
        if ((run==0) || (elapsed_usec < min_usec)) {
            min_usec = elapsed_usec;
            for (i=0 ; i< NUM_EVENTS ; i++) min_values[i] = values [i];
        }
        
    } // end runs
    fprintf (stdout,"\nWall clock time: %lld usecs\n", min_usec);
    
    // output PAPI counters' values
    for (i=0 ; i< NUM_EVENTS ; i++) {
        char EventCodeStr[PAPI_MAX_STR_LEN];
        
        if (PAPI_event_code_to_name(Events[i], EventCodeStr) == PAPI_OK) {
            fprintf (stdout, "%s = %lld\n", EventCodeStr, min_values[i]);
        } else {
            fprintf (stdout, "PAPI UNKNOWN EVENT = %lld\n", min_values[i]);
        }
    }
    /*for (i=0;i<v_size;i++){
    	fprintf(stdout,"%f\n",v[i]);
    }*/
    
    free_vector (&v);
    
    fprintf (stdout,"\nThat's all, folks\n");
    return 0;
}


int alloc_vector (float **v, int N) {
    
    *v = (float *) malloc (N*sizeof(float));
    if (!(*v)) {
        printf("Could not allocate memory for vector!\n");
        return 0;
    }
    return 1;
}

float my_rand (void) {
    double d;
    
    d = drand48 ();
    d *= 1.E2;
    return ((float)d);
}

int ini_vector (float **v, int N) {
    int i;
    float *ptr;
    
    if (!alloc_vector (v, N)) return 0;
    for (i=0 , ptr = (*v) ; i<N ; i++ , ptr++) {
        *ptr = my_rand();
    }
    return 1;
} 

int free_vector (float **v) {
    free (*v);
    *v = NULL;
    return 1;
}
