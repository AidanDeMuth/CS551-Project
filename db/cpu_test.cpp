#include "./timer.hh"
#include "./test.hh"

#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

/* testJoin()
 *
 * Perform a CPU speed test with large table join.
 * 
 * This test maximizes CPU usage by performing a large join with
 * the generate_series function that generates integers as needed
 * and pairs them. It minimizes disk usage as it generates them on
 * the fly, rather than all at once and does not access tables.
 */
double testJoin() {
	pqxx::connection conn = getConnection("testdb");

	
	pqxx::work tx{conn};

	/* Ensure single threaded execution */
	tx.exec("SET max_parallel_workers_per_gather = 0");
	tx.exec("SET max_parallel_workers = 0");

	/* Using a prepared statement will run the query optimizer
	 * but not execute teh statement. That way, the time taken
	 * by the query planner and optimizer is not considered during
     * the join test.
     *
     * These are made on a per-connection basis so they're safe to
     * use between tests
	 */
	std::string prepare = R"(
		PREPARE join_test AS
		SELECT SUM((a * b) % 97)
		FROM generate_series(1, 100000) as a
		CROSS JOIN generate_series(1, 1000) as b;
	)";
	std::string execute = R"(
		EXECUTE join_test;
	)";	

	/* Load the plan */
	tx.exec(prepare);	

    Timer t;
    pqxx::result row = tx.exec(execute);

    double elapsed = t.getms();

	return elapsed;
}


int main(int argc, char *argv[]) {
	std::cout << testJoin() << std::endl;
}
