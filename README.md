# CS551-Project
Cloud Computing Fundamentals project repository .

## Setup
In directory `~/install` give executable permissions:
- `sudo chmod +x ./base.sh`
- `sudo chmod +x ./db_install.sh`

then in `~/install` execute the installation scripts:
- `./base.sh`
- `./db_install.sh install`

## Contents
- `~/install/base.sh`
    - Upgrades OS, installs git, installs c++
    - Sets VIM environment
    
- `~/install/db_install.sh [install | uninstall]`
    - install/uninstalls postgresql and manually creates a cluster
      
- `~/db/DBcontrol.sh [create | drop | list] <db-name>`
    - create/drop a database with a given name, or list databases maintained by the cluster
