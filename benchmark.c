// benchmark.c - Implementation
#include "benchmark.h"
#include "order_book.h"

PerfMonitor perf_monitor = {0};
static double current_start_time = 0;
static char current_metric_name[64] = {0};


// Create log file
void perf_init(const char* log_filename) {
    memset(&perf_monitor, 0, sizeof(PerfMonitor));
    perf_monitor.program_start_time = get_time_ms();
    
    if (log_filename) {
        perf_monitor.log_file = fopen(log_filename, "w");
        if (perf_monitor.log_file) {
            fprintf(perf_monitor.log_file, "timestamp,operation,duration_ms,memory_kb\n");
        }
    }
    
    printf("Performance monitoring initialized\n");
}

// Close log file
void perf_cleanup() {
    if (perf_monitor.log_file) {
        fclose(perf_monitor.log_file);
    }
}

double get_time_ms() {
#ifdef _WIN32
    // Use QueryPerformanceCounter for high precision on Windows
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;
    
    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }
    
    QueryPerformanceCounter(&counter);
    return (double)(counter.QuadPart * 1000.0) / frequency.QuadPart;
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
    } else {
        // Fallback to gettimeofday if clock_gettime fails
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    }
#endif
}

size_t get_memory_usage_kb() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    HANDLE process = GetCurrentProcess();
    
    if (GetProcessMemoryInfo(process, &pmc, sizeof(pmc))) {
        return (size_t)(pmc.WorkingSetSize / 1024);
    }
    return 0;
#else
    // Linux/Unix: Try reading from /proc/self/status first
    FILE* status_file = fopen("/proc/self/status", "r");
    if (status_file) {
        char line[256];
        while (fgets(line, sizeof(line), status_file)) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                size_t rss_kb;
                if (sscanf(line, "VmRSS: %zu kB", &rss_kb) == 1) {
                    fclose(status_file);
                    return rss_kb;
                }
            }
        }
        fclose(status_file);
    }
    
    // Fallback to getrusage
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        #ifdef __APPLE__
            // macOS returns bytes
            return (size_t)(usage.ru_maxrss / 1024);
        #else
            // Linux returns KB
            return (size_t)usage.ru_maxrss;
        #endif
    }
    
    return 0;
#endif
}

// Start timing a block of code logic
void perf_start_timing(const char* metric_name) {
    current_start_time = get_time_ms();
    strncpy(current_metric_name, metric_name, sizeof(current_metric_name) - 1);
    current_metric_name[sizeof(current_metric_name) - 1] = '\0';
}

// Denote end of a block that we have been timing
void perf_end_timing(const char* metric_name) {
    double end_time = get_time_ms();
    double duration = end_time - current_start_time;
    
    // Find or create metric
    PerfMetric *metric = NULL;
    for (int i = 0; i < perf_monitor.metric_count; i++) {
        if (strcmp(perf_monitor.metrics[i].name, metric_name) == 0) {
            metric = &perf_monitor.metrics[i];
            break;
        }
    }
    
    if (!metric && perf_monitor.metric_count < 50) {
        metric = &perf_monitor.metrics[perf_monitor.metric_count++];
        strncpy(metric->name, metric_name, sizeof(metric->name) - 1);
        metric->name[sizeof(metric->name) - 1] = '\0';
        metric->min_time = duration;
        metric->max_time = duration;
    }
    
    if (metric) {
        metric->total_time += duration;
        metric->call_count++;
        if (duration < metric->min_time) metric->min_time = duration;
        if (duration > metric->max_time) metric->max_time = duration;
        
        // Log to file if available
        if (perf_monitor.log_file) {
            fprintf(perf_monitor.log_file, "%.3f,%s,%.4f,%zu\n",
                    end_time - perf_monitor.program_start_time,
                    metric_name, duration, get_memory_usage_kb());
            fflush(perf_monitor.log_file);
        }
    }
}

void perf_log_memory(const char* operation) {
    size_t memory_kb = get_memory_usage_kb();
    double current_time = get_time_ms() - perf_monitor.program_start_time;
    
    printf("[MEMORY] %s: %zu KB at %.3f ms\n", operation, memory_kb, current_time);
    
    if (perf_monitor.log_file) {
        fprintf(perf_monitor.log_file, "%.3f,%s_memory,0,%zu\n", 
                current_time, operation, memory_kb);
        fflush(perf_monitor.log_file);
    }
}

// Display metrics calculated through program running in neat table
void perf_print_summary() {
    double total_runtime = get_time_ms() - perf_monitor.program_start_time;
    
    printf("\n=== PERFORMANCE SUMMARY ===\n");
    printf("Total Runtime: %.2f ms\n", total_runtime);
    printf("Final Memory Usage: %zu KB\n", get_memory_usage_kb());
    printf("\nOperation Performance:\n");
    printf("%-25s %10s %10s %10s %10s %10s\n", 
           "Operation", "Calls", "Total(ms)", "Avg(ms)", "Min(ms)", "Max(ms)");
    printf("------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < perf_monitor.metric_count; i++) {
        PerfMetric *m = &perf_monitor.metrics[i];
        double avg_time = m->call_count > 0 ? m->total_time / m->call_count : 0;
        
        printf("%-25s %10d %10.2f %10.4f %10.4f %10.4f\n",
               m->name, m->call_count, m->total_time, avg_time, m->min_time, m->max_time);
    }
    printf("\n");
}

// Write to CSV
void perf_save_csv(const char* filename) {
    FILE *csv = fopen(filename, "w");
    if (!csv) return;
    
    fprintf(csv, "operation,calls,total_ms,avg_ms,min_ms,max_ms\n");
    for (int i = 0; i < perf_monitor.metric_count; i++) {
        PerfMetric *m = &perf_monitor.metrics[i];
        double avg_time = m->call_count > 0 ? m->total_time / m->call_count : 0;
        fprintf(csv, "%s,%d,%.2f,%.4f,%.4f,%.4f\n",
                m->name, m->call_count, m->total_time, avg_time, m->min_time, m->max_time);
    }
    fclose(csv);
    printf("Performance data saved to %s\n", filename);
}

// Simple benchmark functions
void benchmark_basic_operations() {
    printf("=== BASIC OPERATIONS BENCHMARK ===\n");
    perf_init("basic_benchmark.log");
    
    const int iterations = 10000;
    
    // Test malloc/free performance
    perf_start_timing("malloc_free");
    for (int i = 0; i < iterations; i++) {
        void *ptr = malloc(sizeof(node));
        free(ptr);
    }
    perf_end_timing("malloc_free");
    
    // Test tree operations if available
    perf_start_timing("tree_operations");
    for (int i = 0; i < 1000; i++) {
        node *test_node = malloc(sizeof(node));
        if (test_node) {
            // Initialize node (adjust based on your node structure)
            test_node->price = 1.35000 + (i * 0.00001);
            test_node->volume = 1.0;
            test_node->colour = Red;
            test_node->left = NULL;
            test_node->right = NULL;
            test_node->parent = NULL;
            
            // If you have bidTree available, uncomment:
            // insert_node(&bidTree, test_node);
            // find_best_node(&bidTree);
            
            // For now, just free it
            free(test_node);
        }
    }
    perf_end_timing("tree_operations");
    
    perf_print_summary();
    perf_cleanup();
}

// Memory monitoring function
void monitor_memory_usage(const char* phase) {
    static size_t last_memory = 0;
    size_t current_memory = get_memory_usage_kb();
    
    if (last_memory > 0) {
        long diff = (long)current_memory - (long)last_memory;
        printf("[MEMORY] %s: %zu KB (%+ld KB)\n", phase, current_memory, diff);
    } else {
        printf("[MEMORY] %s: %zu KB\n", phase, current_memory);
    }
    
    last_memory = current_memory;
}