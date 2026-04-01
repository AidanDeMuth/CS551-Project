#!/bin/bash
set -e

# I prob shouldn't be committing this but whatever
CONN="postgresql://postgres:password@localhost:5432/testdb?connect_timeout=5"

DROP="DROP TABLE IF EXISTS test_table;"
CREATE="
	CREATE TABLE test_table(
		id INT,
		balance INT
	);
"

psql "$CONN" -c "$DROP"
psql "$CONN" -c "$CREATE"

# Call some C function
echo "Populating table from bulk_test..."
time ./bulk_test | psql "$CONN" -c "COPY test_table FROM STDIN CSV"

psql "$CONN" -c "$DROP"
