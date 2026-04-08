#include "./timer.hh"
#include "./test.hh"

#include <cstdlib>
#include <iostream>
#include <random>
#include <unistd.h>

const int READ_ID = 1;
const int WRITE_ID = 2;
const int N = 1'000'000;

void run_mixed_workload(pqxx::connection& conn, int read_percentage) {
    std::unique_ptr<pqxx::work> tx = std::make_unique<pqxx::work>(conn);
    Timer t;

    for (int i = 1; i <= N; ++i) {
        int r = rand() % 100;
        if (r < read_percentage) {
            tx->exec("SELECT * FROM test_table WHERE id = "+ std::to_string(READ_ID));
        } else {
            if (rand() % 2 == 0) {
                tx->exec("UPDATE test_table SET balance = balance + 1 WHERE id = "+ std::to_string(WRITE_ID));
            } else {
                tx->exec("INSERT INTO test_table (id, balance) VALUES (3, 4000)");
            }
        }
        if (i % 1000 == 0) {
            tx->commit();
            tx = std::make_unique<pqxx::work>(conn);  // fresh transaction to make it realistic
        }
    }

    tx->commit();
    std::cout << "Read %: " << read_percentage
              << " Time(ms): " << t.getms() << std::endl;
}

int main() {
    srand(time(NULL));

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
    run_mixed_workload(conn, 20);
	run_mixed_workload(conn, 50);
	run_mixed_workload(conn, 80);
    return 0;
}
