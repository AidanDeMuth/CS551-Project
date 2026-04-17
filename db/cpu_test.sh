#!/bin/bash

#!/bin/bash

CONN="postgresql://postgres:password@localhost:5432/testdb"

psql "$CONN" -c "DROP TABLE IF EXISTS test_table;"
psql "$CONN" -c "CREATE TABLE test_table(id INT, balance INT);"

sudo ./cpu_test
perf stat report -i perf.stat

