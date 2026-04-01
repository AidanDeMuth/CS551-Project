#include "./timer.hh"
#include "./test.hh"

#include <iostream>
#include <random>
#include <unistd.h>

int main() {
    const int N = 10'000'000;
	
	Timer t;

	pqxx::connection conn = getConnection("testdb");
	pqxx::work tx{conn};

    // Optional: truncate table before bulk insert
    tx.exec("TRUNCATE TABLE test_table;");

    // Use stream_to for fast bulk insert
    pqxx::stream_to table_stream(tx, "test_table");

    // Seed random generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    
    for (long long i = 1; i <= N; ++i) {
        int id = i;
        int balance = std::rand() % 100000; // random balance

        table_stream << std::make_tuple(id, balance);
    }

    // Send all data to DB
    table_stream.complete();
	tx.commit();

	std::cout << t.getms() << ", generated " << N << " rows." << std::endl;
    return 0;
}
