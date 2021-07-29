#!/bin/bash
# set -x # display execution trace
cd $RVATOM/sw/examples

echo "============| Banner |============"
cd banner
./compile.sh
atomsim banner.elf

cd ..
echo "============| Hello_c |============"
cd hello
./compile_c.sh
atomsim hello.elf

echo "============| Hello_s |============"
./compile_s.sh
atomsim hello.elf

cd ..
echo "============| Fibonacci |============"
cd fibonacci
./compile.sh
atomsim fibonacci.elf

cd ..
echo "============| Pascal |============"
cd pascal
./compile.sh
atomsim pascal.elf

cd ..
echo "============| random |============"
cd random
./compile.sh
atomsim random.elf

cd ..
echo "============| Factorial |============"
cd factorial
./compile.sh
atomsim factorial.elf

cd ..
echo "============| Tower of Hanoi |============"
cd hanoi
./compile.sh
atomsim hanoi.elf
