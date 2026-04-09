#pragma once
#include <sys/resource.h>
#include <fstream>
#include <string>
#include <thread>
#include <cstdio>

inline double get_cpu_time_sec() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    double user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
    double sys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;

    return user + sys;
}

inline long get_memory_kb() {
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

inline long get_total_memory_kb() {
    std::ifstream file("/proc/meminfo");
    std::string key;
    long value;
    std::string unit;
    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") return value;
    }
    return 1;
}

inline double get_ram_percent() {
    return (100.0 * get_memory_kb()) / get_total_memory_kb();
}

inline double compute_cpu_percent(double cpu_time_sec, double wall_time_sec) {
    if (wall_time_sec <= 0) return 0.0;
    int num_cores = std::thread::hardware_concurrency();
    return (cpu_time_sec / (wall_time_sec * num_cores)) * 100.0;
}