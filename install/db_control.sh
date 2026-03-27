#!/bin/bash

install_arg=$1

# Install PostgreSQL

if [[ $# -lt 1 ]]; then
  echo "usage: [install | uninstall]"; exit 1;
fi


if [ "$install_arg" = "install" ]; then
  sudo apt update
  sudo apt upgrade

  sudo apt install postgresql
elif [ "$install_arg" = "uninstall" ]; then
  # Delet config
  sudo apt purge postgresql

  # End procress and delete user
  sudo systemctl stop postgresql
  sudo deluser postgres

  # Get rid of all the little files
  sudo apt autoremove -y
  sudo rm -rf /var/lib/postgresql/
  sudo rm -rf /etc/postgresql/
else
  echo "bad arg"; exit 1
fi
