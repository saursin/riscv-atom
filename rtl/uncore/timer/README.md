# Wishbone Timer
Timer IP with Wishbone B-4 Interface.

## Register MAP

| **Offset** | **Name**   | **Description**
|------------|------------|-----------------------------------
| 0x00       | MTIME      | Machine mode time register [31:0]
| 0x04       | MTIME_H    | Machine mode time register [63:32]
| 0x08       | MTIMECMP   | Machine mode time compare register [31:0]
| 0x0c       | MTIMECMP_H | Machine mode time compare register [63:32]

> Timer interrupt (`mti`) is generated whenever `mtimecmp` >= `mtime`. The `mti` interrupt does not repeat, so the ISR needs to reset the timer compare register (`mtimecmp`) at each timeout.

## Logic Utilization
Yosys was used to analyze Logic utilization for Xilinx 7 series FPGAs

| **Design**    | **LUTS** |
|---------------|----------|
| SPI_core      | 322      |
| SPI_wb        | 530      |
