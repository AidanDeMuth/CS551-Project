#ifndef METRICS_H
#define METRICS_H

long get_memory_kb();
long get_total_memory_kb();
double get_ram_percent();
int get_num_cores();
double compute_cpu_percent(double cpu_time_sec, double wall_time_sec);

#endif