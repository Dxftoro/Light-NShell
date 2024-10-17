#!/bin/bash
echo "Compiling code...";
gcc -o a.out *.c
chmod +x a.out
echo "Code compiled!";
./a.out
rm ./a.out
