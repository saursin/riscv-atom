`ifndef __UTIL_DPI__
`define __UTIL_DPI__

// dpi_logger
import "DPI-C" function void dpi_trace(input string format /*verilator sformat*/);
import "DPI-C" function void dpi_trace_start();
import "DPI-C" function void dpi_trace_stop();

`endif //__UTIL_DPI__