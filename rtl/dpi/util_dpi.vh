`ifndef __UTIL_DPI__
`define __UTIL_DPI__

// dpi_logger
import "DPI-C" function void dpi_logger(input string format /*verilator sformat*/);
import "DPI-C" function void dpi_logger_start();
import "DPI-C" function void dpi_logger_stop();

`endif //__UTIL_DPI__