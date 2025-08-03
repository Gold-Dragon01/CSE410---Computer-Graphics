#!/usr/bin/bash

# Check if a filename is provided
if [ $# -eq 0 ]; then
    echo "Filename not found"
    exit 1
fi

# Extract the filename without extension
filename=$(basename -- "$1")
output="${filename%.*}"

# Compile the OpenGL program
g++ "$1" -o "$output" -lGL -lGLU -lglut

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running the program..."
    ./"$output"
else
    echo "Compilation failed."
    exit 1
fi