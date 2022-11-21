#!/bin/bash

read -p 'How many servers ? ' servers
read -p 'How many clients ? ' clients

echo "localhost slots=$(($servers + $clients + 1))" > hostfile

# Change console by your favorite terminal
mpirun -hostfile hostfile konsole -e gdb -ex run --args bin/afs $servers $clients