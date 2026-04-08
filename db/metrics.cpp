#include "metrics.h"
#include <fstream>
#include <string>
#include <thread>
#include <cstdio> // for sscanf

long get_memory_kb() {
    std::ifstream file("/proc/self/status");
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            long mem;
            sscanf(line.c_str(), "VmRSS: %ld kB", &mem);
            return mem;
        }
    }
    return 0;
}

long get_total_memory_kb() {
    std::ifstream file("/proc/meminfo");
    std::string key;
    long value;
    std::string unit;
    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") return value;
    }
    return 1;
}

double get_ram_percent() {
    long used = get_memory_kb();
    long total = get_total_memory_kb();
    return (100.0 * used) / total;
}

int get_num_cores() {
    // Basic fallback if hardware_concurrency returns 0
    unsigned int n = std::thread::hardware_concurrency();
    return n > 0 ? n : 1;
}

double compute_cpu_percent(double cpu_time_sec, double wall_time_sec) {
    int cores = get_num_cores();
    if (wall_time_sec <= 0) return 0.0;
    return (cpu_time_sec / (wall_time_sec * cores)) * 100.0;
}