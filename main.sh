#!/bin/bash

check_compilation(){
    if [ $? -eq 0 ]; then
        echo "Running experiment..."
    else
        echo "Compilation failed."
        exit 1
    fi
}


if [ -z "$1" ]; then
    echo "No argument supplied."
elif [ -z "$2" ]; then
    echo "Compiling and running src/$1.cpp..."
    g++ -o $1  src/$1.cpp -lGL -lGLU -lglut
    check_compilation
    ./$1
fi