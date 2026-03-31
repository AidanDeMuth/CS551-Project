#!/bin/bash
# Update OS
if grep -q "24.04" /etc/os-release; then
  echo "Already running Ubuntu 24.04"
else
  # Upgrade to 24.04 in theory
  
  sudo apt update 
  sudo apt upgrade -f
  
  if ! grep -q "24.04" /etc/os-release; 
  then echo "Failed to Upgrade OS version"; exit 1; fi;
fi

# Install git
if git --version | grep -q "version"; then
  echo "Git already installed"
else
  sudo apt update
  sudo apt install git
  echo "Successfully installed Git"
fi

# Set VIM default editor and set ~./vimrc file
echo "Setting VIM as default editor"
sudo apt install vim
sudo update-alternatives --install /usr/bin/editor editor /usr/bin/vim 100
sudo update-alternatives --set editor /usr/bin/vim

> ~/.vimrc
echo "set tabstop=4" > ~/.vimrc
echo "set shiftwidth=4" >> ~/.vimrc
echo "set noexpandtab" >> ~/.vimrc

# Install C++, gxx, Make
echo "Installing C++"
if g++ --version | grep -q "version"; then
  echo "C++ already installed!"
else
  sudo apt install build-essential
  echo "Successfully installed C++!"
fi

# Instal libpqxx-dev (library for postgres)
echo "Installing libpqxx-dev"
sudo apt install libpqxx-dev

