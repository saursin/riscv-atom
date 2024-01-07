# Wishbone SPI
SPI IP with Wishbone B-4 Interface.

## Parameters
* **NCS**: Number of hardware chip select lines

## Register MAP

| **Offset** | **Name** | **Description**
|------------|----------|----------------------------
| 0x00       | SCKDIV   | SCK Divisor register
| 0x04       | SCTRL    | SPI Control register
| 0x08       | TDATA    | Transmit data register
| 0x0c       | RDATA    | Recieve data register
| 0x10       | CSCTRL   | Chip Select Control Register
| 0x14       | DCTRL    | Delay Control register


## Register Definitions

### (0x00)  SCKDIV: SCK Divisor register

| **Range** | **Access** | **RstVal** | **Name** | **Description**
|-----------|------------|------------|----------|-----------------
| [31:0]    | RW         | 0x3        | DIVISOR  | SCK Divisor
|           |            |            |          |

**Formula to calculate Divisor**: `Divisor = f_ratio - 1` where `f_ratio = sys_clk_freq / baud_rate`.

### (0x04)  SCTRL: Status & Control Register

| **Range** | **Access** | **RstVal** | **Name** | **Description**
|-----------|------------|------------|----------|-----------------
| [31]      | RO         | 0x0        | BUSY     | SPI Busy
| [30:3]    | -          | -          | -        | Reserved
| [3]       | RW         | 0x0        | LOOP     | Loopback Enable
| [2]       | RW         | 0x0        | END      | Endineness
| [1]       | RW         | 0x0        | PHA      | Phase
| [1]       | RW         | 0x0        | POL      | Polarity
| [0]       | RW         | 0x0        | EN       | SPI Enable
|           |            |            |          |

- END
    - 0 = MSB first
    - 1 = LSB First

- POL
    - 0 = inactive state of SCK is LOW,
    - 1 = inactive state of SCK is high

- PHA:
    - 0 = data sampled on leading edge, shifted on trailing edge
    - 1 = data shifted on leading edge, sampled on trailing edge

### (0x04)  TDATA: Transmit Data Register

| **Range** | **Access** | **RstVal** | **Name** | **Description**
|-----------|------------|------------|----------|-----------------
| [31:8]    | -          | -          | -        | Reserved
| [7:0]     | RW         | 0x0        | DATA     | Transmit Data

### (0x04)  RDATA: Recieve Data Register

| **Range** | **Access** | **RstVal** | **Name** | **Description**
|-----------|------------|------------|----------|-----------------
| [31:8]    | -          | -          | -        | Reserved
| [7:0]     | RW         | 0x0        | DATA     | Recieve Data

### (0x04)  CSCTRL: CS Control Register

| **Range** | **Access** | **RstVal** | **Name** | **Description**
|-----------|------------|------------|----------|-----------------
| [31:24]   | RW         | 0x1        | ACS      | Active Chip Select
| [23:2]    | -          | -          | -        | Reserved
| [1:0]     | RW         | 0x0        | CSMODE   | CS Mode

- CSMODE
    - 2b00: AUTO: Auto Assertion and Dessartion of CS before/after every packet
    - 2b01: Reserved
    - 2b10: Reserved
    - 2b11: DISABLE: Disable hardware control of CS pin

- ACS: (one-hot encoded)
    - when CSMODE == AUTO, Selects which CS to assert/deassart
    - when CSMODE == DISABLE, writing 1 to a bit asserts corresponding CS.

### (0x04)  DCTRL: Delay Control Register

| **Range** | **Access** | **RstVal** | **Name** | **Description**
|-----------|------------|------------|----------|-----------------
| [31:16]   | -          | -          | -        | Reserved
| [15:8]    | RW         | 0x1        | PRCSHD   | Pre CS High Delay
| [7:0]     | RW         | 0x1        | POCSLD   | Post CS Low Delay

Delay values are in terms of number of SCK cycles. i.e. POCSLD == 2 means after CS has been deasserted, there will be a delay equal to 2 sck cycles before starting transaction.

## Logic Utilization
Yosys was used to analyze Logic utilization for Xilinx 7 series FPGAs

| **Design**    | **LUTS** |
|---------------|----------|
| SPI_core      | 322      |
| SPI_wb        | 530      |
