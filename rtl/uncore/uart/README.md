# Wishbone UART
UART IP with Wishbone B-4 Interface
* Reciever & Transmitter can be Enabled/Disabled
* Supports Parity Bit & Dual Stop bit
* Has internal loopback mode for testing


## Parameters

* DEFAULT_DIV = 1

## Register MAP

| **Offset** | **Name** | **Description**
|------------|----------|------------------------------
| 0x00       | RBR      | Receiver Buffer Register     
| 0x00       | THR      | Transmitter Holding Register 
| 0x04       | LCR      | Line Control Register        
| 0x08       | LSR      | Line Status Register         
| 0x0c       | DIV      | Divisor Register             
|            |          |                              

## Register Definitions

### (0x00) RBR: Recieve Buffer Register

| **Range** | **Access** | **RstVal** | **Name** | **Description** 
|-----------|------------|------------|----------|-----------------
| [31:8]    | -          | -          | -        | Reserved        
| [7:0]     | RO         | 0x00       | RXDA     | RX_Data         
|           |            |            |          |                 


### (0x00) THR: Transmit Holding Register

| **Range** | **Access** | **RstVal** | **Name** | **Description** 
|-----------|------------|------------|----------|-----------------
| [31:8]    | -          | -          | -        | Reserved        
| [7:0]     | WO         | 0x00       | TXDA     | TX_Data         
|           |            |            |          |                 


## (0x04) LCR: Line Control Register

| **Range** | **Access** | **RstVal** | **Name** | **Description**                         
|-----------|------------|------------|----------|-----------------------------------------
| [31:9]    | -          | -          | -        | Reserved                                
| [7]       | RW         | 0x0        | LPBK     | Loopback Enable                         
| [6:5]     | -          | -          | -        | Reserved                                
| [4]       | RW         | 0x0        | EPAR     | Even Parity {b0 = Disable, b1 = Enable} 
| [3]       | RW         | 0x0        | PARB     | Parity bits {b0 = Disable, b1 = Enable} 
| [2]       | RW         | 0x0        | STPB     | Stop bits   {b0 = 1 bit, b1 = 2 bits}   
| [1]       | RW         | 0x1        | TXEN     | Tx Enable                               
| [0]       | RW         | 0x1        | RXEN     | Rx Enable                               
|           |            |            |          |                                         


### (0x08) LSR: Line Status Register

| **Range** | **Access** | **RstVal** | **Name** | **Description**                         
|-----------|------------|------------|----------|-----------------------------------------
| [31:4]    | -          | -          | -        | Reserved
| [3]       | RO         | 0x0        | PERR     | Parity Error
| [2]       | RO         | 0x0        | FERR     | Framing Error
| [1]       | RO         | 0x0        | TEMT     | THR Empty  {b1 = Data transmitted}
| [0]       | RO         | 0x0        | RVAL     | RBR valid  {b1 = Complete incoming character has been recieved into RBR}
|           |            |            |          |                                         


## (0x0c) DIV: Divisor Register
    
| **Range** | **Access** | **RstVal**  | **Name** | **Description** 
|-----------|------------|-------------|----------|-----------------
| [31:0]    | RW         | DEFAULT_DIV | DIV      | Divisor
|           |            |             |          |        

**Formula to calculate Divisor**: `Divisor = f_ratio - 2` where `f_ratio = sys_clk_freq / baud_rate`.



## Logic Utilization
Yosys was used to analyze Logic utilization for Xilinx 7 series FPGAs

| **Design**    | **LUTS** |
|---------------|----------|
| simpleuart    | 410      |
| simpleuart_wb | 428      |
| UART_core     | 411      |
| UART          | 506      |
