#include "./test.hh"

#include <pqxx/pqxx>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

const int TARGET_CONNECTIONS = 200;
const int RAMP_UP_DELAY_MS = 10; // Delay between opening connections to avoid thundering herd

std::atomic<int> successful_connections(0);
std::atomic<int> connection_errors(0);

void worker_task(int id) {
    try {
        pqxx::connection conn = getConnection("testdb");
        successful_connections++;
        try {
            pqxx::nontransaction N(conn);
            N.exec("SELECT pg_sleep(10);"); // keep the connection busy for 10 seconds
        } catch (...) {
            // query failure doesn't invalidate connection success
        }
    } catch (const std::exception &e) {
        connection_errors++;
    }
}

int main() {
    std::vector<std::thread> threads;

    std::cout << "Starting saturation test toward " << TARGET_CONNECTIONS << " connections...\n";

    for (int i = 0; i < TARGET_CONNECTIONS; ++i) {
        threads.emplace_back(worker_task, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(RAMP_UP_DELAY_MS));

        if (i % 25 == 0) {
            std::cout << "Attempted: " << i << " | Errors: " << connection_errors << "\n";
        }
    }

    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "\n--- Final Results ---\n";
    std::cout << "Total Successful Connections: " << successful_connections << "\n";
    std::cout << "Total Failed Connections:     " << connection_errors << "\n";

    return 0;
}
