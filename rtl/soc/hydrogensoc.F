////////////////////////////////////////////////////////////////
// HydrogenSoC
////////////////////////////////////////////////////////////////

// Include directories
+incdir+../common
+incdir+../core
+incdir+hydrogensoc

// Topmodule
hydrogensoc/HydrogenSoC.v

// Arbiter
../uncore/wishbone/Arbiter.v
../uncore/wishbone/Arbiter2_wb.v
../uncore/wishbone/Arbiter3_wb.v
../uncore/wishbone/Crossbar5_wb.v
../uncore/wishbone/Priority_encoder.v

// Periphs
../uncore/gpio/IOBuf.v
../uncore/gpio/GPIO.v
../uncore/mem/DualPortRAM_wb.v
../uncore/mem/SinglePortROM_wb.v
../uncore/mem/SinglePortRAM_wb.v
../uncore/uart/UART.v
../uncore/uart/UART_core.v
../uncore/spi/SPI_wb.v
../uncore/spi/SPI_core.v

// Core
../core/AtomRV_wb.v
../core/AtomRV.v
../core/Alu.v
../core/CSR_Unit.v
../core/Decode.v
../core/RegisterFile.v
