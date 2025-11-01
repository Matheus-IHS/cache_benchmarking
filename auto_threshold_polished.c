
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BLOCK 64

#define L1_CACHE_SIZE 48*1024
#define L2_CACHE_SIZE 1280*1024
#define L3_CACHE_SIZE 8192*1024

uint8_t array1[L1_CACHE_SIZE*2];
uint8_t array2[2*L2_CACHE_SIZE];

static inline uint64_t rdtsc() {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline uint64_t probe_native(volatile uint8_t *adrs){
    volatile unsigned long time;
    asm __volatile__(
        "    mfence                \n"
        "    lfence                \n"
        "    rdtsc                 \n"
        "    lfence                \n"
        "    mov %%eax, %%esi      \n"
        "    mov (%1), %%eax       \n"
        "    lfence                \n"
        "    rdtscp                \n"
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
        "    rdtscp                \n"
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
    for(unsigned int i = 0; i < (2*L1_CACHE_SIZE)/(4*1024); i++){
        array[i*(4*1024)] = "m";
    }
}

void fill_array_l2(uint8_t *array){
    for(unsigned int i = 0; i < (2*L2_CACHE_SIZE)/(4*1024); i++){
        array[i*(4*1024)] = "b";
    }
}

void shuffle(long unsigned int *array, size_t n)
{
    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void stall_cycles(uint64_t cycles) {
    uint64_t start = rdtsc();
    uint64_t now;
    do {
        now = rdtsc();
    } while (now - start < cycles);
}

void get_latency_l1(){
    int mix_i;
    int NUM_MEDICOES = 500000;
    volatile uint8_t* addr;
    unsigned int latency[NUM_MEDICOES], latency_dummy[NUM_MEDICOES];
    FILE *LATENCY_FILE;

    for (unsigned int i=0; i < NUM_MEDICOES; i++){
        latency[i] = probe_native(&array1[0]);
        latency_dummy[i] = dummy_probe_native();
    }
    
    int total_latency = 0;
    int subtracted_total_latency = 0;
    for (int i = 0; i < NUM_MEDICOES; i++){total_latency += latency[i];}
    for (int i = 0; i < NUM_MEDICOES; i++){subtracted_total_latency += max(latency[i] - latency_dummy[i], 0);}
    printf("cycles = %f \n", ((float)(total_latency))/((float)NUM_MEDICOES));
    printf("cycles = %f \n", ((float)(subtracted_total_latency))/((float)NUM_MEDICOES));
    
    LATENCY_FILE = fopen("log_auto_threshold_l1", "w");
    for(int i = 0; i < NUM_MEDICOES; i++){
        fprintf(LATENCY_FILE, "%i\n", max(latency[i] - latency_dummy[i],0));
    }
    fclose(LATENCY_FILE);
}

void get_latency_l2(){
    int NUM_MEDICOES = 500000;
    volatile uint8_t lixo;
    unsigned int latency[NUM_MEDICOES], latency_dummy[NUM_MEDICOES];
    unsigned int total_latency = 0;
    unsigned int total_latency_dummy = 0;
    unsigned int subtracted_total_latency = 0;
    unsigned int t1, t2 = 0;
    FILE *LATENCY_FILE;

    long unsigned int num_lines = (2*L1_CACHE_SIZE) / 64;
    long unsigned int indices[num_lines];
    for (long unsigned int i = 0; i < num_lines; i++) indices[i] = i;
    shuffle(indices, num_lines);
 
    for(int i = 0; i < NUM_MEDICOES; i++){
        lixo = array2[0];
        for(int j = 0; j < (2*L1_CACHE_SIZE)/64; j++){
            lixo = array1[indices[j]*64];
        }
        latency[i] = probe_native(&array2[0]);
        latency_dummy[i] = dummy_probe_native();
    }

    for (long int i = 0; i < NUM_MEDICOES; i++){total_latency += latency[i];}
    for (long int i = 0; i < NUM_MEDICOES; i++){subtracted_total_latency += max(latency[i] - latency_dummy[i], 0);}
    printf("cycles = %f \n", ((float)(total_latency))/((float)NUM_MEDICOES));
    printf("cycles = %f \n", ((float)(subtracted_total_latency))/((float)NUM_MEDICOES));
    
    LATENCY_FILE = fopen("log_auto_threshold_l2", "w");
    for(int i = 0; i < NUM_MEDICOES; i++){
        fprintf(LATENCY_FILE, "%i\n", max(latency[i] - latency_dummy[i],0));
    }
    fclose(LATENCY_FILE);
}

void get_latency_l3(){
    int NUM_MEDICOES = 500000;
    volatile uint8_t lixo;
    unsigned int latency[NUM_MEDICOES];
    unsigned int latency_dummy[NUM_MEDICOES];
    unsigned int total_latency = 0;
    unsigned int total_latency_dummy = 0;
    unsigned int subtracted_total_latency = 0;
    unsigned int t1, t2 = 0;
    FILE *LATENCY_FILE;
    
    long unsigned int num_lines = (2*L2_CACHE_SIZE) / 64;
    long unsigned int indices[num_lines];
    for (long unsigned int i = 0; i < num_lines; i++) indices[i] = i;
    shuffle(indices, num_lines);
 
    for(int i = 0; i < NUM_MEDICOES; i++){
        lixo = array1[0];
        for(int j = 0; j < (L2_CACHE_SIZE)/64; j++){
            lixo = array2[indices[j]*64];
        }
        latency[i] = probe_native(&array1[0]);
        latency_dummy[i] = dummy_probe_native();
    }
    for (long int i = 0; i < NUM_MEDICOES; i++){total_latency += latency[i];}
    for (long int i = 0; i < NUM_MEDICOES; i++){subtracted_total_latency += max(latency[i] - latency_dummy[i], 0);}
    printf("cycles = %f \n", ((float)(total_latency))/((float)NUM_MEDICOES));
    printf("cycles = %f \n", ((float)(subtracted_total_latency))/((float)NUM_MEDICOES));
    
    LATENCY_FILE = fopen("log_auto_threshold_l3", "w");
    for(int i = 0; i < NUM_MEDICOES; i++){
        fprintf(LATENCY_FILE, "%i\n", max(latency[i] - latency_dummy[i],0));
    }
    fclose(LATENCY_FILE);
}


int main(){
    printf("long unsigned int: %lu \n", sizeof(long unsigned int));
    get_latency_l1();
    fill_array_l1(array1);
    get_latency_l2();
    fill_array_l2(array2);
    get_latency_l3();
}
