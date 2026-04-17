#include "./timer.hh"
#include "./test.hh"

#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

/* get getBackendPID()
 *
 * We need to query the postgres server to get its backend PID,
 * so this is just a helper to do it given some connection
 */
int getBackendPID(pqxx::connection& conn) {
	pqxx::nontransaction ntx{conn};
	pqxx::result res = ntx.exec("SELECT pg_backend_pid();");
	int pid = res[0][0].as<int>();
	return pid;
}

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

	/* Get the backend PID for perf */
	int backendPID = getBackendPID(conn);
	std::cout << backendPID << std::endl;
	
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

	/* Load the plan + do a warmip */
	tx.exec(prepare);
	
	int pid = fork();
	if (pid < 0) { return -1; }
	if (pid == 0) {
        std::string pidstr = std::to_string(backendPID);
        const char* args[] = {
            "sudo", "perf", "record",
            "-o", "perf_stat.txt",
            "-p", pidstr.c_str(),
            NULL
        }; // useless on t2 micro
        execvp("sudo", (char* const*)args);
        exit(1);
	}

	usleep(500000); // wait for perf to attach

	std::cout << "Perf attached, running query..." << std::endl;
    Timer t;
    pqxx::result row = tx.exec(execute);

    double elapsed = t.getms();

    kill(pid, SIGINT);
    waitpid(pid, NULL, 0);
    usleep(500000); // let perf flush perf.dataconn.close(); 
	return elapsed;
}


int main(int argc, char *argv[]) {
	std::cout << testJoin() << std::endl;
}
