/*
 * Prefetching 		-> ponter chasing
 * code optimization 	-> volatile variables
 * out-of-order		-> memory fencing instructions CPUID
 * core-migrating       -> pin code to a given core
 */




#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define STRIDE     	(64 / sizeof(void *))        	// stride = 1 linha de cache

#define CACHE_SIZE_L1 	(48   * 1024 / sizeof(void *)) 	// ~16KB, cabe no L1 (32KB típico)
#define NUM_BLOCKS_L1  	(CACHE_SIZE_L1 / STRIDE)

#define CACHE_SIZE_L2 	(1280 * 1024 / sizeof(void *))	// ~512KB, cabe no L1 (32KB típico)
#define NUM_BLOCKS_L2  	(CACHE_SIZE_L2 / STRIDE)

static void **array_L1;
static void **array_L2;


static inline uint64_t rdtsc_start() {
    unsigned hi, lo;
    __asm__ __volatile__ (
        "mfence         \n"
	"lfence         \n"
        "rdtsc		\n" 
	: "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline uint64_t rdtsc_end() {
    unsigned hi, lo;
    __asm__ __volatile__ (
        "rdtsc		\n"
	"lfence         \n"
	"mfence        \n"
	: "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

void init_array_L1() {
    // pointer chasing: cada posição aponta para a próxima (stride de 1 linha de cache)
    for (size_t i = 0; i < CACHE_SIZE_L1; i += STRIDE) {
        array_L1[i] = &array_L1[(i + STRIDE) % CACHE_SIZE_L1];
        __asm__ __volatile__ (
            "clflush  (%0)         \n"
     	    :: "r" (&array_L1[i])
        ); 
    }
}

void init_array_L2() {
    // pointer chasing: cada posição aponta para a próxima (stride de 1 linha de cache)
    for (size_t i = 0; i < CACHE_SIZE_L2; i += STRIDE) {
        array_L2[i] = &array_L2[(i + STRIDE) % CACHE_SIZE_L2];
        __asm__ __volatile__ (
            "clflush  (%0)         \n"
     	    :: "r" (&array_L2[i])
        ); 
    }
}

int L2_cache_latency_test() {
    array_L1 = aligned_alloc(64, CACHE_SIZE_L1 * sizeof(void *));
    
    if (!array_L1) { perror("malloc"); return 1; }

    init_array_L1();

    void * volatile *p = &array_L1[0];
    uint64_t start, end;
    
    // aquecer o cache
    for (size_t i = 0; i < CACHE_SIZE_L1; i += STRIDE) {
        p = (void **)*p;
    }

    // medir latência
    start = rdtsc_start();						 // trocar pelo rdtscP
    for (int i = 0; i < NUM_BLOCKS_L1; i++) {
        p = (void **)*p;
    }
    end = rdtsc_end();
    
    float cycles = (float)(end - start)/(float)NUM_BLOCKS_L1; 
    printf("L2 with latency %f cycles\n", (cycles));		

    free(array_L1);
    return 0;
}

int L3_cache_latency_test() {
    array_L2 = aligned_alloc(64, CACHE_SIZE_L2 * sizeof(void *));
    
    if (!array_L2) { perror("malloc"); return 1; }

    init_array_L2();

    void * volatile *p = &array_L2[0];
    uint64_t start, end;

    // aquecer o cache
    for (size_t i = 0; i < CACHE_SIZE_L2; i += STRIDE) {
        p = (void **)*p;
    }

    // medir latência
    start = rdtsc_start();						 // trocar pelo rdtscP
    for (int i = 0; i < NUM_BLOCKS_L2; i++) {
        p = (void **)*p;
    }
    end = rdtsc_end();

    float cycles = (float)(end-start)/(float)NUM_BLOCKS_L2; 
    printf("L3 with latency %f cycles\n", cycles);		

    free(array_L2);
    return 0;
}

int main() {
	printf("sizeof(void *): %ld \n", sizeof(void *));
	L2_cache_latency_test();
	L3_cache_latency_test();

	return 0;
	
}
