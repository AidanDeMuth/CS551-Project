#include "./test.hh"
#include "./timer.hh"

#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <random>

const int NUM_QUERIES = 10'000;

int main() {

    pqxx::connection conn = getConnection("testdb");
    pqxx::work W1(conn);
    {
        pqxx::stream_to table_stream(W1, "test_table");
        for (int i = 1; i <= NUM_QUERIES; ++i) {
            table_stream << std::make_tuple(i, 30000);
        }
        table_stream.complete();
    }
    W1.commit();

    Timer t;
    std::unique_ptr<pqxx::work> W3 = std::make_unique<pqxx::work>(conn);
    for (int i = 0; i < NUM_QUERIES; ++i) {

        pqxx::result R = W3->exec("SELECT balance FROM test_table WHERE id=" + std::to_string(i) + ";");
        if (i % 10000 == 0) {
            W3->commit();
            W3 = std::make_unique<pqxx::work>(conn);
        }
    }
    W3->commit();

    double duration_ms = t.getms();
    std::cout << "Point query transactions per second: " << NUM_QUERIES*1000/duration_ms << ".\n";
    return 0;
}