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

psql "$CONN" -q -c "$DROP"
psql "$CONN" -q -c "$CREATE"

# Call some C function
./bulk_test

psql "$CONN" -q -c "$DROP"
