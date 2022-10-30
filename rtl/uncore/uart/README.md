# Wishbone UART
UART IP with Wishbone B-4 Interface

## Parameters


## Register MAP
```
 Offset | Access  | RstVAl  | Name  | Description
--------+---------+---------+-------+------------------------
  0x00    RO        0x00      RBR     Receiver Buffer Register
  0x00    WO        0x00      THR     Transmitter Holding Register
  0x04    RW        0x00      LCR     Line Control Register
  0x08    RO        0x00      LSR     Line Status Register
  0x0c    RW        0x00      DIV     Divisor Register
```

## Register Definitions

```
1. (0x00) Recieve Buffer Register
   ------------------------------
    [31:8]  Reserved
    [7:0]   RX_Data     (0x00)


2. (0x00) Transmit Holding Register
    ------------------------------
    [31:8]  Reserved
    [7:0]   TX_Data     (0x00)


3. (0x04) Line Control Register
    ------------------------------
    [31:9]  Reserved
    [7]     Loopback Enable
    [6:5]   Reserved
    [4]     Even Parity {b0 = Disable, b1 = Enable}
    [3]     Parity bits {b0 = Disable, b1 = Enable}
    [2]     Stop bits   {b0=1 bit, b1=2 bits}
    [1]     Tx Enable
    [0]     Rx Enable


4. (0x08) Line Status Register
    ------------------------------
    [31:4]  Reserved
    [3]     Parity Error
    [2]     Framing Error
    [1]     THR Empty {1b = Data transmitted}
    [0]     RBR valid {1b = Complete incoming character has been recieved into RBR}


5. (0x0c) Divisor Register
    ------------------------------
    [31:0]  Div
```

## Logic Utilization
Yosys was used to analyze Logic utilization for Xilinx 7 series FPGAs

| **Design**    | **LUTS** |
|---------------|----------|
| simpleuart    | 410      |
| simpleuart_wb | 428      |
| UART_core     | 411      |
| UART          | 506      |
