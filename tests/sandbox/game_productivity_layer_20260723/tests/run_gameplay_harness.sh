#!/bin/bash
if [[ "$OSTYPE" == "darwin"* ]]; then
    clang++ -O3 -std=c++17 gameplay_harness.cpp -I ../include -o gameplay_harness -lpthread -framework CoreFoundation
else
    g++ -O3 -std=c++17 gameplay_harness.cpp -I ../include -o gameplay_harness -lpthread
fi
if [ $? -ne 0 ]; then
    echo "[ERROR] Harness build failed."
    exit 1
fi

./gameplay_harness
