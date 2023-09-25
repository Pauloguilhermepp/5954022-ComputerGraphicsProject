#!/bin/bash

check_compilation(){
    if [ $? -eq 0 ]; then
        echo "Running simulation..."
    else
        echo "Compilation failed."
        exit 1
    fi
}

if [ -z "$1" ]; then
    echo "Compiling and running src/main.cpp..."
    g++ -o main  src/main.cpp -lGL -lGLU -lglut
    check_compilation
    ./main
elif [ "$1" == "format" ]; then
    find . -iname *.hpp -o -iname *.cpp | xargs clang-format -i
else
    echo "Argument \"$1\" is not valid"
fi