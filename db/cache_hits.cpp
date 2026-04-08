#include "./test.hh"

#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <random>

const int N = 4'300'000;
const int NUM_QUERIES = 10'000;

int main() {
    try {
        pqxx::connection conn = getConnection("testdb");
        pqxx::work W(conn);

        W.exec("SELECT pg_stat_reset();");
        W.commit();
        std::cout << "Database stats reset.\n";

        pqxx::work tx{conn};
        tx.exec("TRUNCATE TABLE test_table;");
        pqxx::stream_to table_stream(tx, "test_table");
        for (long long i = 1; i <= N; ++i) {
            table_stream << std::make_tuple(i, 50000);
        }
        table_stream.complete();
        tx.commit();
        std::cout << "Table populated with " << N << " rows.\n";

        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(1, N);

        std::unique_ptr<pqxx::work> W3 = std::make_unique<pqxx::work>(conn);
        for (int i = 0; i < NUM_QUERIES; ++i) {

            int id = dist(rng);
            pqxx::result R = W3->exec("SELECT balance FROM test_table WHERE id=" + std::to_string(id) + ";");
            if (i % 10000 == 0) {
                W3->commit();
                W3 = std::make_unique<pqxx::work>(conn);
            }
        }
        W3->commit();

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