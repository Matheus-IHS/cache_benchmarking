
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BLOCK 64

#define L1_CACHE_SIZE 48*1024
#define L2_CACHE_SIZE 1280*1024
#define L3_CACHE_SIZE 8192*1024

uint8_t array1[L1_CACHE_SIZE];

static inline uint64_t rdtsc() {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline uint64_t  probe_native(volatile uint8_t *adrs){
    volatile unsigned long time;
    asm __volatile__(
        "    mfence                \n"
        "    lfence                \n"
        "    rdtsc                 \n"
        "    lfence                \n"
        "    mov %%eax, %%esi      \n"
        "    mov (%1), %%eax       \n"
        "    lfence                \n"
        "    rdtsc                 \n"
        "    sub %%esi, %%eax      \n"
        "    clflush (%1)          \n"
        "    mfence                \n"
        "    lfence                \n"
        : "=a" (time)
        : "c" (adrs)
        : "%esi", "%edx"
    );
    return time;
}

void get_latency_l1(){
    int mix_i;
    volatile uint8_t* addr;
    long long int latency[L1_CACHE_SIZE];

    for (int i=0; i < L1_CACHE_SIZE; i++){
	//mix_i = ((i * 167) + 13) % L1_CACHE_SIZE;
        latency[i] = probe_native(&array1[i]); /* MEMORY ACCESS TO TIME */
    }
    
    unsigned int total_latency = 0;
    for (int i = 0; i < L1_CACHE_SIZE; i++){total_latency += latency[i];}
    printf("cycles = %d \n", (total_latency)/L1_CACHE_SIZE);
}


int main(){
    get_latency_l1();
}
