#include "./test.hh"
#include "./timer.hh"

#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <random>

const int N = 10'000;
const int NUM_QUERIES = N;

int main() {
    try {
        pqxx::connection conn = getConnection("testdb");
        pqxx::work W(conn);

        W.exec("SELECT pg_stat_reset();");
        W.commit();
        std::cout << "Database stats reset.\n";

        {
            pqxx::work W2(conn);
            W2.exec("DROP TABLE IF EXISTS test_cache;");
            W2.exec("CREATE TABLE test_cache(id SERIAL PRIMARY KEY, value TEXT);");

            for (int i = 0; i < N; ++i) {
                W2.exec0("INSERT INTO test_cache(value) VALUES(" + W2.quote("Value_" + std::to_string(i)) + ");");
            }
            W2.commit();
        }
        std::cout << "Table populated with " << N << " rows.\n";

        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(1, N);

        Timer t;

        for (int i = 0; i < NUM_QUERIES; ++i) {
            pqxx::work W3(conn);
            int id = dist(rng);
            pqxx::result R = W3.exec("SELECT value FROM test_cache WHERE id=" + std::to_string(id) + ";");
            W3.commit();
        }

        double duration_ms = t.getms();
        std::cout << "Point query transactions per second: " << NUM_QUERIES*1000/duration_ms << ".\n";

        pqxx::nontransaction N(conn);
        pqxx::result res = N.exec(
            "SELECT blks_hit, blks_read, "
            "ROUND(blks_hit::numeric/(blks_hit+blks_read)*100,2) AS hit_ratio "
            "FROM pg_stat_database WHERE datname='testdb';"
        );

        int blks_hit = res[0][0].as<int>();
        int blks_read = res[0][1].as<int>();
        std::string hit_ratio = res[0][2].c_str();

        std::cout << "Cache stats after workload:\n";
        std::cout << "Blocks Hit: " << blks_hit << "\n";
        std::cout << "Blocks Read (disk): " << blks_read << "\n";
        std::cout << "Hit Ratio: " << hit_ratio << "%\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}