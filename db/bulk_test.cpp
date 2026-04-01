#include "./timer.hh"
#include "./test.hh"

#include <unistd.h>
#include <iostream>

int main(int argc, char *argv[]) {
	Timer t;

	pqxx::connection conn = getConnection("testdb");
	pqxx::work tx{conn};

	tx.exec("SELECT 1");
	sleep(2);	

	std::cout << t.getms() << std::endl;
}
