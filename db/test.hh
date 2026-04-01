#pragma once

#include <pqxx/pqxx>
#include <string>

pqxx::connection getConnection(std::string dbname) {
    return pqxx::connection{
        "postgresql://postgres:password@localhost:5432/" + dbname + "?connect_timeout=5"
    };
}
