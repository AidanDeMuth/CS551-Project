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

    tx.exec("TRUNCATE TABLE test_table;");

    pqxx::stream_to table_stream(tx, "test_table");
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (long long i = 1; i <= N; ++i) {
        int id = i;
        int balance = std::rand() % 100000; // random balance

        table_stream << std::make_tuple(id, balance);
    }

    table_stream.complete();
	tx.commit();

	std::cout << t.getms() << std::endl;
    return 0;
}
