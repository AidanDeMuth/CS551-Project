#!/bin/bash

INSTALL_ARG=$1
POSTGRES_PASS="password"
PG_VERSION=$(psql --version | awk '{print $3}' | cut -d. -f1)

# Install PostgreSQL

if [[ $# -lt 1 ]]; then
  echo "usage: [install | uninstall]"; exit 1;
fi


if [ "$INSTALL_ARG" = "install" ]; then
  sudo apt update
  sudo apt upgrade

  # Make a cluster
  if ! pg_lsclusters | grep -q main; then
    sudo pg_createcluster 16 main
  fi

  # Give the postgres user a dummy password
  sudo apt install postgresql-16 postgresql-contrib
  sudo -u postgres psql -c "ALTER USER postgres WITH PASSWORD '${POSTGRES_PASS}'"

elif [ "$INSTALL_ARG" = "uninstall" ]; then
  # Delet config
  sudo apt purge postgresql

  # End procress and delete user
  sudo systemctl stop postgresql
  sudo deluser postgres

  # Get rid of all the little files
  sudo apt purge -y postgresql-16 postgresql-contrib postgresql-common
  sudo apt autoremove -y
  sudo rm -rf /var/lib/postgresql/
  sudo rm -rf /etc/postgresql/
else
  echo "bad arg"; exit 1
fi
