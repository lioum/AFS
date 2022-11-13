#!/bin/bash

read -p 'How many servers ? ' servers
read -p 'How many clients ? ' clients

echo "localhost slots=$(($servers + $clients + 1))" > hostfile

mpirun -hostfile hostfile ./bin/afs $servers $clients
