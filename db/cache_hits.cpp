#include "./test.hh"

#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <random>
#include <chrono>

int main() {
    try {
        pqxx::connection conn = getConnection("testdb");
        pqxx::work W(conn);

        W.exec("SELECT pg_stat_reset();");
        W.commit();
        std::cout << "Database stats reset.\n";

        //TODO delete
        std::cout << "Current stats:\n";
        pqxx::nontransaction N1(conn);
        pqxx::result res1 = N1.exec(
            "SELECT blks_hit, blks_read, "
            "ROUND(blks_hit::numeric/(blks_hit+blks_read)*100,2) AS hit_ratio "
            "FROM pg_stat_database WHERE datname='testdb';"
        );
        int a = res1[0][0].as<int>();
        int b = res1[0][1].as<int>();

        std::cout << "Blocks Hit: " << a << "\n";
        std::cout << "Blocks Read (disk): " << b << "\n";
        // TODO end delete

        {
            pqxx::work W2(conn);
            W2.exec("DROP TABLE IF EXISTS test_cache;");
            W2.exec("CREATE TABLE test_cache(id SERIAL PRIMARY KEY, value TEXT);");

            for (int i = 0; i < 10000; ++i) {
                W2.exec0("INSERT INTO test_cache(value) VALUES(" + W2.quote("Value_" + std::to_string(i)) + ");");
            }
            W2.commit();
        }
        std::cout << "Table populated with 10000 rows.\n";

        // 4) Simulate SQL workload (point queries)
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(1, 10000);
        const int NUM_QUERIES = 10000;

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < NUM_QUERIES; ++i) {
            pqxx::work W3(conn);
            int id = dist(rng);
            pqxx::result R = W3.exec("SELECT value FROM test_cache WHERE id=" + std::to_string(id) + ";");
            W3.commit();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        std::cout << "Executed " << NUM_QUERIES << " point queries in " << duration_ms << " ms.\n";

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