# SPI


```
1. (0x00)   SCKDIV:     SCK Divisor register
    [31:0]      RW  0x3     Divisor

2. (0x04)   SCKMODE:    SCK Mode Register
    [31:2]      -   -       Reserved
    [1]         RW  0x0     Polarity    b0 = inactive state of SCK is LOW,
                                        b1 = inactive state of SCK is high
    [0]         RW  0x0     Phase       b0 = data sampled on leading edge, shifted on trailing edge
                                        b1 = data shifted on leading edge, sampled on trailing edge

3. (0x08)   SPICTRL:    SPI Control Register


4. (0x0c)   CSREG:      CHIP Select Register
    [31:NCS]    -   -       Reserved
    [NCS-1:0]   RW  0x0     CS Pin to be toggled

5. (0x10)   CSCTRL:     CHIP Select Control Register
    [31:0]      RW  0x0     Reserved
    [1:0]       RW  0x0     Chip Select Mode:   b0 = AUTO deassert/assert in every frame
                                                b1 = HOLD keep CS asserted after 1st frame, deassert when CSCTRL re-written
                                                b2 = DISABLE hw control of CS pin

6. (0x14)   DCTRL:      Delay Control Register
    [31:24]     -   -       Reserved
    [23:16]     RW  0x1     Interframe delay
    [15:8]      RW  0x1     SCK-CS Delay
    [7:0]       RW  0x1     CS-SCK Delay

7.  (0x18)  FMT:        Format Register
    [31:20]     -   -       Reserved
    [18:16]     RW  0x8     Bits per frame
    [2]         RW  0x0     Endineness          b0 = MSB first
                                                b1 = LSB First
    [1:0]       RW  0x0     Protocol            b0 = single
                                                b1 = dual
                                                b2 = quad
    
    

```