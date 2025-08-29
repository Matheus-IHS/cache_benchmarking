#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define STRIDE     (64 / sizeof(void *))        // stride = 1 linha de cache
#define CACHE_SIZE_L1 (16 * 1024 / sizeof(void *)) // ~16KB, cabe no L1 (32KB típico)
#define NUM_BLOCKS_L1  (CACHE_SIZE_L1 / STRIDE)

static void **array_L1;


static inline uint64_t rdtsc() {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

void init_array_L1() {
    // pointer chasing: cada posição aponta para a próxima (stride de 1 linha de cache)
    for (size_t i = 0; i < CACHE_SIZE_L1; i += STRIDE) {
        array_L1[i] = &array_L1[(i + STRIDE) % CACHE_SIZE_L1];
    }
}

int L1_cache_latency_test() {
    array_L1 = aligned_alloc(64, CACHE_SIZE_L1 * sizeof(void *));
    uint64_t latency[NUM_BLOCKS_L1];
    double block;
    
    if (!array_L1) { perror("malloc"); return 1; }

    init_array_L1();

    void * volatile *p = &array_L1[0];
    uint64_t start, end;

    // aquecer o cache
    for (size_t i = 0; i < CACHE_SIZE_L1; i += STRIDE) {
        p = (void **)*p;
    }

    // medir latência
    for (int i = 0; i < NUM_BLOCKS_L1; i++) {
		start = rdtsc();						 // trocar pelo rdtscP
        p = (void **)*p;
        end = rdtsc();
        latency[i] = end-start;                  // Pode estar interferindo na medição. colocar mfence antes do start
    }

	for (int i = 0; i < NUM_BLOCKS_L1; i++) {;
		//if(cycles >= 7){
		printf("Victim acessed Block # %u with latency %lu cycles\n", i, latency[i]);
		
	}

    free(array_L1);
    return 0;
}

int main() {
	int a;
	
	a = 0;
	a = L1_cache_latency_test();
	return a;
	
}
