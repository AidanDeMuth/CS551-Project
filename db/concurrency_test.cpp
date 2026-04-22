#include "./test.hh"
#include "./timer.hh"
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex print_mutex;

void insertRows(int thread_id, int num_rows, pqxx::connection &conn) {
  pqxx::work tx{conn};
  for (int i = 0; i < num_rows; i++) {
    float temp = -20.0f + (rand() % 1400) / 10.0f;
    float humidity = (rand() % 1001) / 10.0f;
    float rain = (rand() % 1001) / 10.0f;
    float snow = (rand() % 1001) / 10.0f;

    tx.exec_params("INSERT INTO weather (temp_f, humidity, rain_pct, snow_pct) "
                   "VALUES ($1, $2, $3, $4)",
                   temp, humidity, rain, snow);
  }
  tx.commit();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " inserted " << num_rows << " rows\n";
}

void avgQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;
  pqxx::result res = tx.exec(R"(
		SELECT AVG(temp_f), MIN(temp_f), MAX(temp_f), COUNT(*) FROM weather
	)");
  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " query over " << res.size()
  //           << " rows took " << elapsed << " ms\n";
}
void updateHotRows(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;

  // Update the same 100 rows repeatedly to force contention
  for (int i = 1; i <= 100; i++) {
    tx.exec_params("UPDATE weather SET humidity = humidity + 1 WHERE id = $1",
                   i);
  }

  tx.commit();
  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " updated hot rows in " << elapsed
  //          << " ms\n";
}
void deleteOldRows(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;

  pqxx::result res =
      tx.exec("DELETE FROM weather WHERE temp_f < 0 RETURNING id");

  tx.commit();
  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " deleted " << res.size()
  //           << " rows in " << elapsed << " ms\n";
}
void pointLookupQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;

  int random_id = 1 + rand() % 50000;
  pqxx::result res =
      tx.exec_params("SELECT * FROM weather WHERE id = $1", random_id);

  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " point lookup took " << elapsed
  //           << " ms\n";
}
void rangeScanQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;

  int start = rand() % 50000;
  pqxx::result res = tx.exec_params(
      "SELECT * FROM weather WHERE id BETWEEN $1 AND $2", start, start + 2000);

  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " range scan returned " << res.size()
  //           << " rows in " << elapsed << " ms\n";
}
void countDistinctQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;

  pqxx::result res =
      tx.exec("SELECT COUNT(DISTINCT ROUND(temp_f)) FROM weather");

  double elapsed = t.getms();
  std::lock_guard<std::mutex> lock(print_mutex);

  // std::cout << "Thread " << thread_id << " distinct count took " << elapsed
  //           << " ms\n";
}
void windowFunctionQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;

  pqxx::result res = tx.exec(R"(
        SELECT id, temp_f,
               AVG(temp_f) OVER (ORDER BY id ROWS BETWEEN 50 PRECEDING AND CURRENT ROW),
               SUM(rain_pct) OVER (ORDER BY id ROWS BETWEEN 50 PRECEDING AND CURRENT ROW)
        FROM weather
    )");

  double elapsed = t.getms();
  std::lock_guard<std::mutex> lock(print_mutex);

  // std::cout << "Thread " << thread_id << " window query took " << elapsed
  //           << " ms\n";
}
void createIndexQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;

  tx.exec("CREATE INDEX IF NOT EXISTS idx_temp ON weather(temp_f)");
  tx.commit();

  double elapsed = t.getms();
  std::lock_guard<std::mutex> lock(print_mutex);

  // std::cout << "Thread " << thread_id << " created index in " << elapsed
  //           << " ms\n";
}

void bulkInsertCopy(int thread_id, pqxx::connection &conn) {
  Timer t;
  pqxx::work tx{conn};

  for (int i = 0; i < 5000; i++) {
    float temp = -20.0f + (rand() % 1400) / 10.0f;
    float humidity = (rand() % 1001) / 10.0f;
    float rain = (rand() % 1001) / 10.0f;
    float snow = (rand() % 1001) / 10.0f;

    tx.exec_params("INSERT INTO weather (temp_f, humidity, rain_pct, snow_pct) "
                   "VALUES ($1, $2, $3, $4)",
                   temp, humidity, rain, snow);
  }

  tx.commit();
  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " bulk inserted 5000 rows in "
  //           << elapsed << " ms\n";
}

void groupQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;
  pqxx::result res = tx.exec(R"(
	SELECT ROUND(temp_f), COUNT(*) FROM weather GROUP BY ROUND(temp_f) ORDER BY ROUND(temp_f)
	)");
  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " query over " << res.size()
  //           << " rows took " << elapsed << " ms\n";
}
void filterQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;
  pqxx::result res = tx.exec(R"(
		SELECT * FROM weather WHERE temp_f > 90 ORDER BY humidity DESC LIMIT 50
	)");
  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " query over " << res.size()
  //           << " rows took " << elapsed << " ms\n";
}
void randomQuery(int thread_id, pqxx::connection &conn) {
  pqxx::work tx{conn};
  Timer t;
  pqxx::result res = tx.exec(R"(
        SELECT id, temp_f,
            AVG(temp_f) OVER (ORDER BY id ROWS BETWEEN 9 PRECEDING AND CURRENT ROW) AS rolling_avg
        FROM weather
    )");
  double elapsed = t.getms();

  std::lock_guard<std::mutex> lock(print_mutex);
  // std::cout << "Thread " << thread_id << " query over " << res.size()
  //           << " rows took " << elapsed << " ms\n";
}

void worker(int thread_id, int num_rows) {
  try {
    pqxx::connection conn = getConnection("testdb");
    insertRows(thread_id, num_rows, conn);

    switch (thread_id) {
    case 0:
      filterQuery(thread_id, conn);
      randomQuery(thread_id, conn);
      break;
    case 1:
      updateHotRows(thread_id, conn);
      pointLookupQuery(thread_id, conn);
      break;
    case 2:
      rangeScanQuery(thread_id, conn);
      countDistinctQuery(thread_id, conn);
      break;
    case 3:
      deleteOldRows(thread_id, conn);
      windowFunctionQuery(thread_id, conn);
      break;
    case 4:
      bulkInsertCopy(thread_id, conn);
      createIndexQuery(thread_id, conn);
      break;
    }
  } catch (const std::exception &e) {
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cerr << "Thread " << thread_id << " failed: " << e.what() << "\n";
  }
}

int main(int argc, char *argv[]) {
  int num_threads = 5;   // default
  int rows_each = 10000; // default, 4*250 = 1000 total

  // allow overriding from command line: ./multithread_test 8 500
  if (argc >= 2)
    num_threads = std::atoi(argv[1]);
  if (argc >= 3)
    rows_each = std::atoi(argv[2]);

  srand(time(0));

  // Make sure table exists
  /*
	pqxx::connection setup_conn = getConnection("testdb");

  pqxx::work setup_tx{setup_conn};
  setup_tx.exec(R"(
        CREATE TABLE IF NOT EXISTS weather (
            id        SERIAL PRIMARY KEY,
            temp_f    FLOAT NOT NULL,
            humidity  FLOAT NOT NULL,
            rain_pct  FLOAT NOT NULL,
            snow_pct  FLOAT NOT NULL
        )
    )");
  setup_tx.commit();

  std::cout << "Spawning " << num_threads << " threads, " << rows_each
            << " rows each (" << num_threads * rows_each << " total)\n";
*/
  Timer t;

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; i++) {
    threads.emplace_back(worker, i, rows_each);
  }
  for (auto &th : threads) {
    th.join();
  }

  double elapsed = t.getms();
  std::cout << "\nAll threads done in " << elapsed << " ms\n";
  std::cout << "Throughput: " << (num_threads * rows_each) / (elapsed / 1000.0)
            << " rows/sec\n";

  return 0;
}
