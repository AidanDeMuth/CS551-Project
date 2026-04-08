#include "./timer.hh"
#include "./test.hh"

#include <cstdlib>
#include <iostream>
#include <random>
#include <unistd.h>

const int READ_ID = 1;
const int WRITE_ID = 2;

void run_mixed_workload(int N, int read_percentage) {
    Timer t;

    for (int i = 1; i <= N; ++i) {
        int r = rand() % 100;

        if (r < read_percentage) {
            tx.exec("SELECT * FROM test_table WHERE id = "+ std::to_string(READ_ID));
            run_read(tx, i);
        } else {
            tx.exec("UPDATE test_table SET balance = balance + 1 WHERE id = "+ std::to_string(WRITE_ID));
            run_write(tx, i);
        }
    }

    tx.commit();
    std::cout << "Read %: " << read_percentage
              << " Time(ms): " << t.getms() << std::endl;
}

int main() {
    const int N = 1'000'000;
    pqxx::connection conn = getConnection("testdb");
	pqxx::work tx{conn};
    pqxx::stream_to table_stream(tx, "test_table");

    // Populate initial table
    for (int i = 1; i <= N/2; ++i) {
        table_stream << std::make_tuple(READ_ID, 30000);
    }
    for (int i = 1; i <= N/2; ++i) {
        table_stream << std::make_tuple(WRITE_ID, 30000);
    }
    table_stream.complete();
	tx.commit();

    // Run mixed read/write workloads
    run_mixed_workload(N, 20);
	run_mixed_workload(N, 50);
	run_mixed_workload(N, 80);
    return 0;
}
