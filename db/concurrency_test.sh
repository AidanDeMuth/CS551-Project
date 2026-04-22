#!/bin/bash


CPP_SCRIPT_NAME="concurrency_test"
CONN="postgresql://postgres:password@localhost:5432/testdb"



## QUERIES - Multiconnection test uses weather table

DROP="DROP TABLE IF EXISTS weather;"
CREATE="
        CREATE TABLE weather(
            id			SERIAL PRIMARY KEY,
			temp_f  	FLOAT NOT NULL,
			humidity	FLOAT NOT NULL,
			rain_pct	FLOAT NOT NULL,
			snow_pct	FLOAT NOT NULL
        );
		"


## Execute, and ensure table cleanup

echo "Dropping table before test"
psql "$CONN" -q -c "$DROP" > /dev/null 2>/dev/null

echo "Creating table before test"
psql "$CONN" -q -c "$CREATE"

./$CPP_SCRIPT_NAME

echo "Dropping table after test"
psql "$CONN" -q -c "$DROP"
