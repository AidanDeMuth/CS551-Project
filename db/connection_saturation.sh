#!/bin/bash


CPP_SCRIPT_NAME="connection_saturation"
CONN="postgresql://postgres:password@localhost:5432/testdb"


## QUERIES

# This test requires no table queries

echo "Setting max_connections to 2000..."
psql "$CONN" -c "ALTER SYSTEM SET max_connections = 2000;"
psql "$CONN" -c "ALTER SYSTEM SET superuser_reserved_connections = 5;"

echo "Restarting PostgreSQL..."
sudo systemctl restart postgresql

## Execute

./$CPP_SCRIPT_NAME

