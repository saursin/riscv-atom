# Spartan6-Mini Board

by FPGATECHSOLUTIONS

![](https://m.media-amazon.com/images/I/81vmncc9IRL._SX522_.jpg)

The board includes:

- FPGA: Xilinx Spartan-6 (XC6SLX9-TQ144)

- OSCILLATOR - 12MHz

- FLASH - 8M SPI Flash (M25P80)

- On board USB JTAG/Serial

- 8 LEDs

- 4 DIP SWITCHES

- 4 PUSH BUTTONS

- 71 USER I/O

- 1 RGB LED

HydrogenSoC Mapping:

- gpio[3:0] are mapped to leds[3:0]

- cpu reset ("rst_i") is mapped to pushbutton[0]

- 12 Mhz clock from the oscillator is provided to the "clk_i" port.
