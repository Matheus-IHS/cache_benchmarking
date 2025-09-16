
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BLOCK 64

#define L1_CACHE_SIZE 48*1024          /*511: limite máximo até segfault*/
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
        "    mfence                \n"
        "    lfence                \n"
        : "=a" (time)
        : "c" (adrs)
        : "%esi", "%edx"
    );
    return time;
}

static inline uint64_t  dummy_probe_native(){
    volatile unsigned long time;
    asm __volatile__(
        "    mfence                \n"
        "    lfence                \n"
        "    rdtsc                 \n"
        "    lfence                \n"
        "    mov %%eax, %%esi      \n"
        "    lfence                \n"
        "    rdtsc                 \n"
        "    sub %%esi, %%eax      \n"
        "    mfence                \n"
        "    lfence                \n"
        : "=a" (time)
        :: "%esi", "%edx"
    );
    return time;
}

unsigned int max(long long int a, long long int b){
    if(a>b){
        return a;
    }
    else{
        return b;
    }
}

void fill_array_l1(uint8_t *array){
    for(unsigned int i = 0; i < L1_CACHE_SIZE/(4*1024); i++){
        array[i*(4*1024)] = "m";
    }
}

void get_latency_l1(){
    int mix_i;
    int NUM_MEDICOES = 1000;
    volatile uint8_t* addr;
    long long int latency[NUM_MEDICOES];
    long long int latency_dummy[NUM_MEDICOES];

    for (unsigned int i=0; i < NUM_MEDICOES; i++){
	//mix_i = ((i * 167) + 13) % L1_CACHE_SIZE;
	//printf("endereco: %p \n", &array1[i]);
        latency[i] = probe_native(&array1[0]); /* MEMORY ACCESS TO TIME */
    }
    for (unsigned int i=0; i < NUM_MEDICOES; i++){
	//mix_i = ((i * 167) + 13) % L1_CACHE_SIZE;
        latency_dummy[i] = dummy_probe_native(); /* MEMORY ACCESS TO TIME */
    }
    
    unsigned int total_latency = 0;
    unsigned int subtracted_total_latency = 0;
    for (long int i = 0; i < NUM_MEDICOES; i++){total_latency += latency[i];}
    for (long int i = 0; i < NUM_MEDICOES; i++){subtracted_total_latency += max(latency[i] - latency_dummy[i], 0);}
    printf("cycles = %f \n", ((float)(total_latency))/((float)NUM_MEDICOES));
    printf("cycles = %f \n", ((float)(subtracted_total_latency))/((float)NUM_MEDICOES));
}

void get_latency_l2(){
   int NUM_MEDICOES = 1000;
   volatile uint8_t lixo;
   int total_latency = 0;
   int total_latency_dummy = 0; 
   int t1, t2 = 0;

   for(int i = 0; i < NUM_MEDICOES; i++){
       lixo = array1[0];
       for(int j = 0; j < L1_CACHE_SIZE/64; j++){
           lixo = array1[j*64];
       }
       t1 = probe_native(&array1[0]);
       t2 = dummy_probe_native();
       total_latency += t1;
       total_latency_dummy += t2;
       printf("t1 = %i, t2 = %i, t1 - t2: %i \n", t1, t2, t1 - t2);
   }
   printf("cycles = %f \n", ((float)(total_latency))/((float)NUM_MEDICOES));
   printf("cycles = %f \n", ((float)(total_latency - total_latency_dummy))/((float)NUM_MEDICOES));
}

int main(){
    get_latency_l1();
    fill_array_l1(array1);
    get_latency_l2();
}
