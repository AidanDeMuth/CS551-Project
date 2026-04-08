#pragma once
#include <fstream>
#include <string>
#include <thread>
#include <cstdio>

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

inline int get_num_cores() {
    unsigned int n = std::thread::hardware_concurrency();
    return n > 0 ? n : 1;
}

inline double compute_cpu_percent(double cpu_time_sec, double wall_time_sec) {
    if (wall_time_sec <= 0) return 0.0;
    return (cpu_time_sec / (wall_time_sec * get_num_cores())) * 100.0;
}