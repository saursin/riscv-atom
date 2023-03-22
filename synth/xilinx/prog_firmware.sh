#!/usr/bin/bash
# $1: firmware path
OFFSET=0xc0000
echo "Programming firmware at $OFFSET"
openFPGALoader --fpga-part xc6slx9tq144 -f -o $OFFSET $1