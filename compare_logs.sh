#!/bin/bash

first_file="server_folders/logs/1.logs"
for i in server_folders/logs/*.logs; do
    diff -q "$i" $first_file;
done