#!/bin/bash
# set -x # display execution trace
cd $RVATOM/sw/examples

echo "============| Banner |============"
cd banner
./compile_hydrogensoc.sh
atomsim banner.elf

cd ..
echo "============| Hello_c |============"
cd hello
./compile_c_hydrogensoc.sh
atomsim hello.elf

echo "============| Hello_s |============"
./compile_s_hydrogensoc.sh
atomsim hello.elf

cd ..
echo "============| Fibonacci |============"
cd fibonacci
./compile_hydrogensoc.sh
atomsim fibonacci.elf

cd ..
echo "============| Pascal |============"
cd pascal
./compile_hydrogensoc.sh
atomsim pascal.elf

cd ..
echo "============| Random |============"
cd random
./compile_hydrogensoc.sh
atomsim random.elf

cd ..
echo "============| Factorial |============"
cd factorial
./compile_hydrogensoc.sh
atomsim factorial.elf

cd ..
echo "============| Tower of Hanoi |============"
cd hanoi
./compile_hydrogensoc.sh
atomsim hanoi.elf