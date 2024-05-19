# Wishbone Timer
Timer IP with Wishbone B-4 Interface.

## Register MAP

| **Offset** | **Name**   | **Description**
|------------|------------|-----------------------------------
| 0x00       | MTIME      | Machine mode time register [31:0]
| 0x04       | MTIME_H    | Machine mode time register [63:32]
| 0x08       | MTIMECMP   | Machine mode time compare register [31:0]
| 0x0c       | MTIMECMP_H | Machine mode time compare register [63:32]

## Register Definitions

### (0x00)  MTIME: Machine mode time register lo

| **Range** | **Access** | **RstVal** | **Name** | **Description**
|-----------|------------|------------|----------|-----------------
| [31:0]    | R          | 0x0        | TIME_LO  | Current time value (lo)
|           |            |            |          |

### (0x04)  MTIMEH: Machine mode time register hi

| **Range** | **Access** | **RstVal** | **Name** | **Description**
|-----------|------------|------------|----------|-----------------
| [31:0]    | R          | 0x0        | TIME_HI  | Current time value (hi)
|           |            |            |          |

### (0x04)  MTIMECMP: Machine mode time compare register lo

| **Range** | **Access** | **RstVal** | **Name**   | **Description**
|-----------|------------|------------|------------|-----------------
| [31:0]    | RW         | 0xffffffff | TIMECMP_LO | Current time compare value (lo)
|           |            |            |            |

### (0x0c)  MTIMECMPH: Machine mode time compare register hi

| **Range** | **Access** | **RstVal** | **Name**   | **Description**
|-----------|------------|------------|------------|-----------------
| [31:0]    | RW         | 0xffffffff | TIMECMP_HI | Current time compare value (hi)
|           |            |            |            |

> Timer expires whenever `mtime` >= `mtimecmp` and interrupt is generated
> Timer interrupt does not repeat, so the ISR needs to reset the timer compare register (`mtimecmp`) at each timeout.
> writing a large value (e.g. 0xffffffffffffffff) to `mtimecmp` clears the interrupt.

## Logic Utilization
Yosys was used to analyze Logic utilization for Xilinx 7 series FPGAs

| **Design**    | **LUTS** |
|---------------|----------|
| SPI_core      | 322      |
| SPI_wb        | 530      |
