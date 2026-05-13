#!/bin/bash

for f in tests/*.csv; do
    echo "--------------------------------"
    echo "Testing $f"
    ./csvreader "$f"
done