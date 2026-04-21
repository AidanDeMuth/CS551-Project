#!/bin/bash

#!/bin/bash

CONN="postgresql://postgres:password@localhost:5432/testdb"

psql "$CONN" -c "DROP TABLE IF EXISTS test_table;"
psql "$CONN" -c "CREATE TABLE test_table(id INT, balance INT);"

echo "CPU large join test execution time:"
sudo ./cpu_test

