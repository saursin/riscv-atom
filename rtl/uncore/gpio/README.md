# Wishbone GPIO
GPIO IP with Wishbone B-4 Interface.
* Configurable number of pins (upto 32)
* Each pin configurable as input or output

## Parameters
* **N**: Number of GPIO Pins. ( 32 >= N > 0)

## Register MAP

| **Offset** | **Name** | **Description**            |
|------------|----------|----------------------------|
| 0x00       | DAT      | Data Register              |
| 0x04       | TSC      | Tri-state contol Register  |
| 0x08       | INC      | Interrupt Control Register |
|            |          |                            |


## Register Definitions

### (0x00)  DAT: Data Register

| **Range** | **Access** | **RstVal** | **Name** | **Description** 
|-----------|------------|------------|----------|-----------------
| [31:N]    | -          | -          | -        | Reserved
| [N-1:0]   | RW         | 0x0        | DATA     | Data
|           |            |            |          | 

* If a pin is in output mode:
  - Reads to DAT return pin value
  - Writes to DAT changes pin value
* If a pin is in input mode:
  - Reads to DAT return pin value
  - Writes to DAT has no effect
* Bit == 0: LOW, Bit==1: HIGH


### (0x04)  TSC: Tri-State Control Register

| **Range** | **Access** | **RstVal** | **Name** | **Description** 
|-----------|------------|------------|----------|-----------------
| [31:N]    | -          | -          | -        | Reserved
| [N-1:0]   | RW         | 0x0        | TSV      | Tri-state values
|           |            |            |          | 

* BIT == 0: configure pin as output, BIT == 1: configure pin as input.


### (0x08)  INC: Interrupt Control Register

| **Range** | **Access** | **RstVal** | **Name** | **Description** 
|-----------|------------|------------|----------|-----------------
| [31:0]    | -          | -          | -        | Not Implemented
|           |            |            |          | 
  


## Logic Utilization
Yosys was used to analyze Logic utilization for Xilinx 7 series FPGAs

| **N** | **LUTS** |
|-------|----------|
| 8     | 179      |
| 16    | 212      |
| 24    | 245      |
| 32    | 278      |
|       |          |