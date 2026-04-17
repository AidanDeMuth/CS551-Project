#!/usr/bin/env bash
set -euo pipefail

DB_NAME="testdb"
CONN="postgresql://postgres:password@localhost:5432/${DB_NAME}?connect_timeout=5"
RESULT_DIR="results_$(date +%Y%m%d_%H%M%S)"

mkdir -p "$RESULT_DIR"

sudo apt update
sudo apt install -y g++ make postgresql postgresql-contrib libpq-dev libpqxx-dev

sudo systemctl start postgresql || true
sudo systemctl enable postgresql || true
sudo -u postgres psql -c "ALTER USER postgres WITH PASSWORD 'password';" || true
sudo -u postgres psql -tc "SELECT 1 FROM pg_database WHERE datname='${DB_NAME}'" | grep -q 1 || sudo -u postgres createdb "${DB_NAME}"

chmod +x base.sh db_install.sh DBcontrol.sh run_test.sh || true

./base.sh || true
./db_install.sh install || true
./DBcontrol.sh create "${DB_NAME}" || true

g++ -std=c++17 -Wall -Wextra bulk_insert.cpp -o bulk_insert -lpqxx -lpq
g++ -std=c++17 -Wall -Wextra cpu_test.cpp -o cpu_test -lpqxx -lpq
g++ -std=c++17 -Wall -Wextra mixed_rw.cpp -o mixed_rw -lpqxx -lpq
g++ -std=c++17 -Wall -Wextra queries_per_second.cpp -o queries_per_second -lpqxx -lpq
g++ -std=c++17 -Wall -Wextra cache_hits.cpp -o cache_hits -lpqxx -lpq

chmod +x bulk_insert cpu_test mixed_rw queries_per_second cache_hits

./run_test.sh bulk_insert | tee "${RESULT_DIR}/result_bulk.txt"
./cpu_test | tee "${RESULT_DIR}/result_join.txt"
./run_test.sh mixed_rw | tee "${RESULT_DIR}/result_mixed_rw.txt"
./run_test.sh queries_per_second | tee "${RESULT_DIR}/result_qps.txt"

psql "$CONN" -c "DROP TABLE IF EXISTS test_table;"
psql "$CONN" -c "CREATE TABLE test_table (id SERIAL PRIMARY KEY, balance INT);"
./cache_hits | tee "${RESULT_DIR}/result_cache_hits.txt"
psql "$CONN" -c "DROP TABLE IF EXISTS test_table;"

pgbench -i -s 10 "$CONN" | tee "${RESULT_DIR}/pgbench_init.txt"
{
pgbench -c 10 -j 2 -T 30 "$CONN"
pgbench -c 25 -j 2 -T 30 "$CONN"
pgbench -c 50 -j 4 -T 30 "$CONN"
pgbench -c 75 -j 4 -T 30 "$CONN"
pgbench -c 100 -j 4 -T 30 "$CONN"
pgbench -c 150 -j 4 -T 30 "$CONN"
} | tee "${RESULT_DIR}/result_connection_saturation.txt"

echo "Results saved in folder: $RESULT_DIR. Copy this folder from the VM to local machine"
