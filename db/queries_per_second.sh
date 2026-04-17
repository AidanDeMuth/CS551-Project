#!/bin/bash
set -e

CPP_SCRIPT_NAME="queries_per_second"
CONN="postgresql://postgres:password@localhost:5432/testdb?connect_timeout=5"

## Queries

DROP="DROP TABLE IF EXISTS test_table;"
CREATE="
	CREATE TABLE test_table(
		id SERIAL PRIMARY KEY,
		balance INT
	);
"

## Execution

psql "$CONN" -q -c "$DROP" > /dev/null 2>/dev/null
psql "$CONN" -q -c "$CREATE"

./$CPP_SCRIPT_NAME

psql "$CONN" -q -c "$DROP"
