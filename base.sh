#!/bin/bash

# Update OS
if grep -q "24.04" /etc/os-release; then
  echo "Already running Ubuntu 24.04"
else
  # Upgrade to 24.04 in theory
  
  sudo apt update 
  sudo apt upgrade -f
  
  if ! grep -q "24.04" /etc/os-release; then exit 1; fi;
fi

# Install git
if grep -q "version" git --version; then
  echo "Git already installed"
else
  sudo apt update
  sudo apt install git
fi

# Set VIM default editor
sudo update-alternatives --install /usr/bin/editor editor /usr/bin/vim 100
sudo update-alternatives --set editor /usr/bin/vim
