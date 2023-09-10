#!/usr/bin/bash
# $1: bitstream path
openFPGALoader --fpga-part xc6slx9tqg144 -f --verify $1 -r
