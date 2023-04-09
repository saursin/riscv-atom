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
../uncore/wishbone/arbiter.v
../uncore/wishbone/arbiter2_wb.v
../uncore/wishbone/arbiter3_wb.v
../uncore/wishbone/priority_encoder.v

// Periphs
../uncore/gpio/IOBuf.v
../uncore/gpio/GPIO.v
../uncore/mem/DualPortRAM_wb.v
../uncore/mem/SinglePortRAM_wb.v
../uncore/uart/UART.v
../uncore/uart/UART_core.v

// Core
../core/AtomRV.v
../core/AtomRV_wb.v
../core/Alu.v
../core/CSR_Unit.v
../core/Decode.v
../core/RegisterFile.v
