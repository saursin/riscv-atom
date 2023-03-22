#!/usr/bin/bash
# $1: bitstream path
openFPGALoader --fpga-part xc6slx9tq144 -f --verify $1 -r