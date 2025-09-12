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

static void **array_L1;

static inline uint64_t rdtsc() {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

void init_array_L1(int cache_size, int stride) {
    // pointer chasing: cada posição aponta para a próxima (stride de 1 linha de cache)
    for (size_t i = 0; i < cache_size; i += stride) {
        array_L1[i] = &array_L1[(i + stride) % cache_size];
    }
}

int L2_cache_latency_test(int cache_size, int num_blocks, int stride) {
    array_L1 = aligned_alloc(64, cache_size * sizeof(void *));
    if (!array_L1) { perror("malloc"); return 1; }

    init_array_L1(cache_size, stride);

    void * volatile *p = &array_L1[0];
    uint64_t start, end;

    // aquecer o cache
    for (size_t i = 0; i < cache_size; i += stride) {
        p = (void **)*p;
    }

    // medir latência
    start = rdtsc();						 // trocar pelo rdtscP
    for (int i = 0; i < num_blocks; i++) {
        p = (void **)*p;
    }
    end = rdtsc();

    float cycles = (float)(end - start)/(float)num_blocks; 
    printf("L2 with latency %f cycles\n", cycles);		

    free(array_L1);
    return 0;
}

int main(int argc, char *argv[]) {
        int stride = 64 / sizeof(void *);        	// stride = 1 linha de cache
	int cache_size = 4*1024;                	// ~16KB, cabe no L1 (32KB típico)
        int num_blocks;
	if (argc < 2) {
           printf("Nenhum argumento fornecido\n");
           return 1;
        }
	cache_size = atoi(argv[1])/sizeof(void*);
        num_blocks = (cache_size / stride);
	L2_cache_latency_test(cache_size, num_blocks, stride);

	return 0;
	
}
