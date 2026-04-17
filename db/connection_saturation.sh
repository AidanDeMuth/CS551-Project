#!/bin/bash


CPP_SCRIPT_NAME="connection_saturation"
CONN="postgresql://postgres:password@localhost:5432/testdb"


## QUERIES

# This test requires no table queries

## Execute

./$CPP_SCRIPT_NAME

