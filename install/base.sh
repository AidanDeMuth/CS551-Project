#!/bin/bash

# Update OS to 24.04 in theory and package manager
echo 'Updating Ubuntu OS to version 24.04'
sudo apt update -y -qq 
sudo apt upgrade -f -y -qq

# Just bail if the OS cannot upgrade  
if ! grep -q "24.04" /etc/os-release; 
then echo "Failed to Upgrade OS version"; exit 1; fi;

# Install git
echo 'Installing git'
sudo apt install git -y -qq

# Set VIM default editor and set ~./vimrc file
echo "Setting VIM as default editor"
sudo apt install vim -y -qq
sudo update-alternatives --install /usr/bin/editor editor /usr/bin/vim 100
sudo update-alternatives --set editor /usr/bin/vim

echo 'Configuring vimrc file'
> ~/.vimrc
echo "set tabstop=4" > ~/.vimrc
echo "set shiftwidth=4" >> ~/.vimrc
echo "set noexpandtab" >> ~/.vimrc

# Install C++, gxx, Make
echo "Installing C++"
sudo apt install build-essential -y -qq

# Install libpqxx-dev (library for postgres)
echo "Installing libpqxx-dev"
sudo apt install libpqxx-dev -y -qq

# Install Make
echo "Installing make"
sudo apt install make -y -qq
