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
sudo -u postgres psql -tc "SELECT 1 FROM pg_database WHERE datname='${DB_NAME}'" | grep -q 1 || sudo -u postgres createdb "${DB_NAME}" || true

chmod +x ../install/*.sh || true
chmod +x *.sh || true

./base.sh || true
./db_install.sh install || true
./DBcontrol.sh create "${DB_NAME}" || true

make || true

reset_db() {
  sudo -u postgres psql -d postgres <<SQL
SELECT pg_terminate_backend(pid)
FROM pg_stat_activity
WHERE datname='${DB_NAME}' AND pid <> pg_backend_pid();
DROP DATABASE IF EXISTS ${DB_NAME};
CREATE DATABASE ${DB_NAME};
SQL
}

echo "-------------\nRunning Tests\n-------------"

reset_db
./bulk_insert.sh | tee "${RESULT_DIR}/result_bulk.txt"

reset_db
./cache_hits.sh | tee "${RESULT_DIR}/result_cache_hits.txt"

reset_db
./concurrency_test.sh | tee "${RESULT_DIR}/result_concurrency.txt"

reset_db
./cpu_test.sh | tee "${RESULT_DIR}/result_cpu.txt"

reset_db
./mixed_rw.sh | tee "${RESULT_DIR}/result_mixed_rw.txt"

reset_db
./queries_per_second.sh | tee "${RESULT_DIR}/result_qps.txt"

reset_db
./connection_saturation.sh | tee "${RESULT_DIR}/result_connection_saturation.txt"

reset_db

run_pgbench_once() {
  local clients="$1"
  local threads="$2"
  pgbench -i -s 10 "$CONN" > /dev/null 2>&1
  set +e
  local output
  output=$(pgbench -c "$clients" -j "$threads" -T 30 "$CONN" 2>&1)
  local status=$?
  set -e
  printf "%s\n" "$output"
  return $status
}

MAX_FILE="${RESULT_DIR}/result_max_connections.txt"
: > "$MAX_FILE"

last_ok=0
first_fail=0

for clients in 10 25 50 75 100 125 150 175 200; do
  threads=2
  [[ "$clients" -ge 50 ]] && threads=4

  echo "===== ${clients} clients =====" | tee -a "$MAX_FILE"

  set +e
  output=$(run_pgbench_once "$clients" "$threads")
  status=$?
  set -e

  printf "%s\n\n" "$output" | tee -a "$MAX_FILE"

  if [[ $status -eq 0 ]]; then
    last_ok=$clients
  else
    first_fail=$clients
    break
  fi
done

if [[ $first_fail -gt 0 ]]; then
  for ((clients=last_ok+1; clients<first_fail; clients++)); do
    threads=2
    [[ "$clients" -ge 50 ]] && threads=4

    echo "===== ${clients} clients =====" | tee -a "$MAX_FILE"

    set +e
    output=$(run_pgbench_once "$clients" "$threads")
    status=$?
    set -e

    printf "%s\n\n" "$output" | tee -a "$MAX_FILE"

    if [[ $status -eq 0 ]]; then
      last_ok=$clients
    else
      break
    fi
  done
fi

echo "MAX_SUCCESSFUL_CONNECTIONS=${last_ok}" | tee -a "$MAX_FILE"

make clean || true

echo "Results saved in folder: $RESULT_DIR. Copy this folder from the VM to local machine"