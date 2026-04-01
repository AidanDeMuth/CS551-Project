#include "./timer.hh"
#include "./test.hh"

#include <unistd.h>
#include <iostream>

// Also consider doing CREATE UNCLOGGED TABLE, better for DISK, WAL, CPU, and JOINS

int main(int argc, char *argv[]) {
	Timer t;

	pqxx::connection conn = getConnection("testdb");
	pqxx::work tx{conn};

	std::string query = R"(
		SELECT a.id, b.id 
		FROM generate_series(1, 10000) as a(id)
		JOIN generate_series(1, 10000) as b(id)
		ON b.id BETWEEN a.id AND a.id + 999
	)";
	tx.exec(query);
	tx.commit();

	std::cout << t.getms() << std::endl;
}
