#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

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

void fill_array1(uint8_t *array){
    for(int i = 0; i < (2*L1_CACHE_SIZE)/(4*1024); i++){
        array[i*(4*1024)] = "m";
    }
}

void fill_array2(uint8_t *array){
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

float get_cpu_nominal_freq() {
    FILE *fp;
    char buffer[256];
    char *line;
    float frequency = 0.0;

    fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        perror("Error opening /proc/cpuinfo");
        exit(1);
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        line = strstr(buffer, "@");
	if (line != NULL) {
            sscanf(line, "@%fGHz", &frequency);
            break;
        }
    }

    fclose(fp);
    return frequency;
}


static long get_sysfs_freq(int core) {
    char path[128];
    snprintf(path, sizeof(path),
             "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", core);

    FILE *f = fopen(path, "r");
    if (!f) {
        // try cpuinfo_cur_freq
        snprintf(path, sizeof(path),
                 "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_cur_freq", core);
        f = fopen(path, "r");
        if (!f) return -1;
    }

    long khz;
    if (fscanf(f, "%ld", &khz) != 1) khz = -1;
    fclose(f);
    return khz;
}

volatile void get_latency(int num_medicoes, uint8_t *cache_filling_array, int array_size, char *log_file_name){
    volatile uint8_t lixo;
    volatile uint8_t probe_address[1];
    volatile int latency[num_medicoes], latency_dummy[num_medicoes];
    int total_latency = 0;
    int total_latency_dummy = 0;
    int subtracted_total_latency = 0;
    float actual_frequency;
    float nominal_frequency;
    FILE *LATENCY_FILE;

    actual_frequency  = get_sysfs_freq(sched_getcpu())/1000;
    nominal_frequency = get_cpu_nominal_freq()*1000;

    for(int i = 0; i < num_medicoes; i++){
        lixo = probe_address[0];
        for(int j = 0; j < (array_size)/64; j++){
            lixo = cache_filling_array[j*64];
        }
        latency[i] = probe_native(&probe_address[0]);
        latency_dummy[i] = dummy_probe_native();
        latency[i] = latency[i]*actual_frequency/nominal_frequency;
        latency_dummy[i] = latency_dummy[i]*actual_frequency/nominal_frequency;
    }

    for (long int i = 0; i < num_medicoes; i++){total_latency += latency[i];}
    for (long int i = 0; i < num_medicoes; i++){subtracted_total_latency += max(latency[i] - latency_dummy[i], 0);}
    printf("Collected %i samples. \n", num_medicoes);
    printf("Raw mean: %.1f cycles.\n", ((float)(subtracted_total_latency))/((float)num_medicoes));
    printf("Saved to %s. \n\n", log_file_name);
    
    LATENCY_FILE = fopen(log_file_name, "w");
    for(int i = 0; i < num_medicoes; i++){
        fprintf(LATENCY_FILE, "%i\n", max(latency[i] - latency_dummy[i],0));
    }
    fclose(LATENCY_FILE);
}


int main(){ 
    get_latency(10000, NULL, 0, "log_latencia_cache_l1");
    fill_array1(array1);
    get_latency(10000, array1, L1_CACHE_SIZE, "log_latencia_cache_l2");
    fill_array2(array2);
    get_latency(10000, array2, L1_CACHE_SIZE+L2_CACHE_SIZE, "log_latencia_cache_l3");
}
