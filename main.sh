#!/bin/bash

check_compilation(){
    if [ $? -eq 0 ]; then
        echo "Running simulation..."
    else
        echo "Compilation failed."
        exit 1
    fi
}

echo "Compiling and running src/main.cpp..."
g++ -o main  src/main.cpp -lGL -lGLU -lglut
check_compilation
./main
