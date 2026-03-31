#include "./DBmanager.hh"
#include <iostream>

pqxx::connection getConnection(std::string dbname) {
	return pqxx::connection{
		"postgresql://postgres:password@localhost:5432/" + dbname + "?connect_timeout=5"
	};
}

int main(int argc, char *argv[]) {
	pqxx::connection conn = getConnection("testdb");
	pqxx::work tx{conn};
	pqxx::row r = tx.exec("SELECT 1")[0];
	tx.commit();
   
	std::cout << r[0].as<int>() << std::endl;
}
