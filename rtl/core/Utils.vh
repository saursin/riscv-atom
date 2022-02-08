`ifndef __UTILS_H__
`define __UTILS_H__

/////////////////////////////////////////////////////////////
// Function-like utility macros

`define CLOG2(x)    $clog2(x)

`define ABS(x)      (($signed(x) < 0) ? (-$signed(x)) : (x));

`define MIN(x, y)   (((x) < (y)) ? (x) : (y))

`define MAX(x, y)   (((x) > (y)) ? (x) : (y))


/////////////////////////////////////////////////////////////
// Macros for Verilator

`define TRACING_ON  /* verilator tracing_on */

`define TRACING_OFF /* verilator tracing_off */

`define UNUSED_VAR(x) always @(x) begin end

`define UNUSED_PARAM(x)  /* verilator lint_off UNUSED */ \
                         localparam  __``x = x; \
                         /* verilator lint_on UNUSED */

`define UNUSED_PIN(x)  /* verilator lint_off PINCONNECTEMPTY */ \
                       . x () \
                       /* verilator lint_on PINCONNECTEMPTY */

`define ASSERT(cond, msg) \
    assert(cond) else $error msg

`endif // __UTILS_H__
