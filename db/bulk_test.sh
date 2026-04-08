#!/bin/bash
set -e

# I prob shouldn't be committing this but whatever
CONN="postgresql://postgres:password@localhost:5432/testdb?connect_timeout=5"

DROP="DROP TABLE IF EXISTS test_table;" > /dev/null
CREATE="
	CREATE TABLE test_table(
		id INT,
		balance INT
	);
"

psql "$CONN" -c "$DROP" > /dev/null
psql "$CONN" -c "$CREATE" > /dev/null

# Call some C function
./bulk_test

psql "$CONN" -c "$DROP" > /dev/null
