// benchmark.h - Performance monitoring system
#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <psapi.h>
    // Link required Windows libraries
    #pragma comment(lib, "psapi.lib")
    #pragma comment(lib, "kernel32.lib")
#else
    #include <sys/time.h>
    #include <sys/resource.h>
    #include <unistd.h>
#endif

// Performance metrics structure
typedef struct {
    char name[64];
    double total_time;
    int call_count;
    double min_time;
    double max_time;
    long memory_used;
} PerfMetric;

// Performance monitor
typedef struct {
    PerfMetric metrics[50];
    int metric_count;
    double program_start_time;
    FILE *log_file;
} PerfMonitor;

// Global performance monitor
extern PerfMonitor perf_monitor;

// Function prototypes
void perf_init(const char* log_filename);
void perf_cleanup();
double get_time_ms();
size_t get_memory_usage_kb();
void perf_start_timing(const char* metric_name);
void perf_end_timing(const char* metric_name);
void perf_log_memory(const char* operation);
void perf_print_summary();
void perf_save_csv(const char* filename);
void monitor_memory_usage(const char* phase);

// Convenience macros
#define PERF_TIME_BLOCK(name) \
    perf_start_timing(name); \
    do

#define PERF_TIME_END() \
    while(0); \
    perf_end_timing(__FUNCTION__)

#define PERF_FUNCTION_START() perf_start_timing(__FUNCTION__)
#define PERF_FUNCTION_END() perf_end_timing(__FUNCTION__)

#endif // BENCHMARK_H