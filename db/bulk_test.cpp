#include "./test.hh"

#include <iostream>
#include <random>

int main() {
    const int N = 10'000'000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 100000);

    for (int i = 1; i <= N; i++) {
        std::cout << i << "," << dist(rng) << "\n";
    }

    std::cerr << "Generated " << N << " rows." << std::endl;

    return 0;
}
