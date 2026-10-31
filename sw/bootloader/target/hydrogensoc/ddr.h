#pragma once

#ifdef SOC_EN_DDR
// Waits for DDR calibration, prints the status and runs a quick test (never stops the boot)
void ddr_check();

// Prints the size and calibration state
void ddr_info();
#endif
