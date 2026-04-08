#!/bin/bash
set -e

CPP_SCRIPT_NAME=$1
if [[ $# -lt 1 ]]; then
  echo "usage: [bulk_insert | mixed_rw | queries_per_second]"; exit 1;
fi

# I prob shouldn't be committing this but whatever
CONN="postgresql://postgres:password@localhost:5432/testdb?connect_timeout=5"

DROP="DROP TABLE IF EXISTS test_table;"
CREATE="
	CREATE TABLE test_table(
		id INT,
		balance INT
	);
"

psql "$CONN" -q -c "$DROP" > /dev/null 2>/dev/null
psql "$CONN" -q -c "$CREATE"

# Call some C function
./$CPP_SCRIPT_NAME

psql "$CONN" -q -c "$DROP"
