#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <sched.h>
cpu_set_t  mask;


#define BLOCK 64

#define L1_CACHE_SIZE 48*1024
#define L2_CACHE_SIZE 1280*1024
#define L3_CACHE_SIZE 8192*1024

uint8_t array1[2*L1_CACHE_SIZE];
uint8_t array2[2*L2_CACHE_SIZE];


static inline int probe_native(volatile uint8_t *adrs){
    volatile int time;
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

static inline int  dummy_probe_native(){
    volatile int long time;
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

int max(int a, int b){
    if(a>b){
        return a;
    }
    else{
        return b;
    }
}

void fill_array_l1(uint8_t *array){
    for(int i = 0; i < (2*L1_CACHE_SIZE)/(4*1024); i++){
        array[i*(4*1024)] = "m";
    }
}

void fill_array_l2(uint8_t *array){
    for(int i = 0; i < (2*L2_CACHE_SIZE)/(4*1024); i++){
        array[i*(4*1024)] = "b";
    }
}

void shuffle(int *array, size_t n)
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

volatile void get_latency_l1(){
    int mix_i;
    int NUM_MEDICOES = 500000;
    volatile uint8_t* addr;
    int latency[NUM_MEDICOES], latency_dummy[NUM_MEDICOES];
    FILE *LATENCY_FILE;

    for (int i=0; i < NUM_MEDICOES; i++){
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

volatile void get_latency_l2(int num_medicoes, uint8_t *cache_filling_array, int array_size, char *log_file_name){
    volatile uint8_t lixo;
    volatile uint8_t probe_address[1];
    volatile int latency[num_medicoes], latency_dummy[num_medicoes];
    int total_latency = 0;
    int total_latency_dummy = 0;
    int subtracted_total_latency = 0;
    FILE *LATENCY_FILE;

    for(int i = 0; i < num_medicoes; i++){
        lixo = probe_address[0];
        for(int j = 0; j < (array_size)/64; j++){
            lixo = array1[j*64];
        }
        latency[i] = probe_native(&probe_address[0]);
        latency_dummy[i] = dummy_probe_native();
    }

    //for (long int i = 0; i < num_medicoes; i++){total_latency += latency[i];}
    for (long int i = 0; i < num_medicoes; i++){subtracted_total_latency += max(latency[i] - latency_dummy[i], 0);}
    //printf("cycles = %f \n", ((float)(total_latency))/((float)num_medicoes));
    printf("cycles = %f \n", ((float)(subtracted_total_latency))/((float)num_medicoes));
    
    LATENCY_FILE = fopen(log_file_name, "w");
    for(int i = 0; i < num_medicoes; i++){
        fprintf(LATENCY_FILE, "%i\n", max(latency[i] - latency_dummy[i],0));
    }
    fclose(LATENCY_FILE);
}

void get_latency_l3(){
    int NUM_MEDICOES = 500000;
    volatile uint8_t lixo;
    int latency[NUM_MEDICOES];
    int latency_dummy[NUM_MEDICOES];
    int total_latency = 0;
    int total_latency_dummy = 0;
    int subtracted_total_latency = 0;
    int t1, t2 = 0;
    FILE *LATENCY_FILE;
    
    for(int i = 0; i < NUM_MEDICOES; i++){
        lixo = array1[0];
        for(int j = 0; j < (2*L2_CACHE_SIZE)/64; j++){
            lixo = array2[j*64];
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

void assign_to_this_core(int core_id){
    CPU_ZERO(&mask);
    CPU_SET(core_id, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
}

int main(){
    fill_array_l2(array2);
    
    printf("core 0 \n");
    assign_to_this_core(0);
    sleep(1);
    get_latency_l2(1000, array2, 2*L2_CACHE_SIZE, "log_auto_threshold_l3");
    
    printf("core 1 \n");
    assign_to_this_core(1);
    sleep(1);
    get_latency_l2(1000, array2, 2*L2_CACHE_SIZE, "log_auto_threshold_l3");
    
    printf("core 2 \n");
    assign_to_this_core(2);
    sleep(1);
    get_latency_l2(1000, array2, 2*L2_CACHE_SIZE, "log_auto_threshold_l3");
    
    printf("core 3 \n");
    assign_to_this_core(3);
    sleep(1);
    get_latency_l2(1000, array2, 2*L2_CACHE_SIZE, "log_auto_threshold_l3");
    
    printf("core 4 \n");
    assign_to_this_core(4);
    sleep(1);
    get_latency_l2(1000, array2, 2*L2_CACHE_SIZE, "log_auto_threshold_l3");
    
    printf("core 5 \n");
    assign_to_this_core(5);
    sleep(1);
    get_latency_l2(1000, array2, 2*L2_CACHE_SIZE, "log_auto_threshold_l3");
    
    printf("core 6 \n");
    assign_to_this_core(6);
    sleep(1);
    get_latency_l2(1000, array2, 2*L2_CACHE_SIZE, "log_auto_threshold_l3");
    
    printf("core 7 \n");
    assign_to_this_core(7);
    sleep(1);
    get_latency_l2(1000, array2, 2*L2_CACHE_SIZE, "log_auto_threshold_l3");
}
