#!/bin/bash
echo "Compiling code...";
gcc -o a.out main.c
chmod +x a.out
echo "Code compiled!";
./a.out
rm ./a.out
