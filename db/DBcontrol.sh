#!/bin/bash

# Arg 1: create/delete database
# Arg 2: DB name

OPTION="$1"
DB_NAME="$2"
DB_USER="postgres"

# Usage
if [[ $# -lt 1 ]]; then
    echo "Usage: ./createDB.sh [create | drop | list] <db-name>"; exit 1;
fi


## Single Arg Options
if [[ $# == 1 ]] && [[ "$OPTION" == "list" ]]; then
    echo "Listing databases"
    sudo -u "$DB_USER" psql -P pager=off -l
    exit 0  
fi

## Multi Arg Options
if [[ $# -lt 2 ]]; then
    echo "Improper args, check usage"; exit 1;
fi

if [[ "$OPTION" = "create" ]]; then
    echo "Creating database ${DB_NAME}"
    sudo -u "$DB_USER" createdb "$DB_NAME"
elif [[ "$OPTION" = "drop" ]]; then
    echo "Dropping database ${DB_NAME}"
    sudo -u "$DB_USER" dropdb "$DB_NAME"
fi

