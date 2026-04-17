#!/usr/bin/env bash
set -euo pipefail

DB_NAME="testdb"
CONN="postgresql://postgres:password@localhost:5432/${DB_NAME}?connect_timeout=5"
RESULT_DIR="results_$(date +%Y%m%d_%H%M%S)"

mkdir -p "$RESULT_DIR"

## Update OS packages and script dependencies
sudo apt update
sudo apt install -y g++ make postgresql postgresql-contrib libpq-dev libpqxx-dev

## Install POSTGRES and initialize cluster
sudo systemctl start postgresql || true
sudo systemctl enable postgresql || true
sudo -u postgres psql -c "ALTER USER postgres WITH PASSWORD 'password';" || true
sudo -u postgres psql -tc "SELECT 1 FROM pg_database WHERE datname='${DB_NAME}'" | grep -q 1 || sudo -u postgres createdb "${DB_NAME}"

## Give executable permissions to shell scripts
# Install Directory
chmod +x ../install/*.sh

# DB Directory (current director)
chmod +x *.sh || true

## Run base installation for OS and Postgres
./base.sh || true
./db_install.sh install || true
./DBcontrol.sh create "${DB_NAME}" || true     # Use a test db named "testdb"

## Create executables
make

## Execute
echo "-------------\nRunning Tests\n-------------"
./bulk_insert.sh | tee "${RESULT_DIR}/result_bulk.txt" 
./cache_hits.sh | tee "${RESULT_DIR}/result_cache_hits.txt" 
./concurrency_test.sh | tee "${RESULT_DIR}/result_concurrency.txt" 
./cpu_test.sh | tee "${RESULT_DIR}/result_cpu.txt"
./mixed_rw.sh | tee "${RESULT_DIR}/result_mixed_rw.txt"
./queries_per_second.sh | tee "${RESULT_DIR}/result_qps.txt" 

## Make this last?
./connection_saturation.sh | tee "${RESULT_DIR}/result_connection_saturation.txt" 

## Internal pg tests
pgbench -i -s 10 "$CONN" | tee "${RESULT_DIR}/pgbench_init.txt"
{
pgbench -c 10 -j 2 -T 30 "$CONN"
pgbench -c 25 -j 2 -T 30 "$CONN"
pgbench -c 50 -j 4 -T 30 "$CONN"
pgbench -c 75 -j 4 -T 30 "$CONN"
pgbench -c 100 -j 4 -T 30 "$CONN"
pgbench -c 150 -j 4 -T 30 "$CONN"
} | tee "${RESULT_DIR}/result_connection_saturation.txt"

## Remove exes
make clean


echo "Results saved in folder: $RESULT_DIR. Copy this folder from the VM to local machine"
