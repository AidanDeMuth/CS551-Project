#include <pqxx/pqxx>
#include <cstring>
#include <string>

pqxx::connection getConnection(std::string dbname);
