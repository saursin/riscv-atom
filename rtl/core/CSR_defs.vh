`ifndef __CSR_DEFS_VH__
`define __CSR_DEFS_VH__

////////////////////////////////////////////////////////////////////
// CSR Address definitions

// Unprivileged Floating-Point CSRs
`define CSR_fflags          21'h001     // Floating-Point Accrued Exceptions.
`define CSR_frm             21'h002     // Floating-Point Dynamic Rounding Mode.
`define CSR_fcsr            21'h003     // Floating-Point Control and Status Register (frm + fflags).

// Unprivileged Counter/Timers
`define CSR_cycle           12'hC00     // Cycle counter for RDCYCLE instruction.
`define CSR_time            12'hC01     // Timer for RDTIME instruction.
`define CSR_instret         12'hC02     // Instructions-retired counter for RDINSTRET instruction.
`define CSR_hpmcounter3     12'hC03     // Performance-monitoring counter.
`define CSR_hpmcounter4     12'hC04     // Performance-monitoring counter.
`define CSR_hpmcounter31    12'hC1F     // Performance-monitoring counter.
`define CSR_cycleh          12'hC80     // Upper 32 bits of cycle, RV32 only.
`define CSR_timeh           12'hC81     // Upper 32 bits of time, RV32 only.
`define CSR_instreth        12'hC82     // Upper 32 bits of instret, RV32 only.
`define CSR_hpmcounter3h    12'hC83     // Upper 32 bits of hpmcounter3, RV32 only.
`define CSR_hpmcounter4h    12'hC84     // Upper 32 bits of hpmcounter4, RV32 only.
`define CSR_hpmcounter31h   12'hC9F     // Upper 32 bits of hpmcounter31, RV32 only.



// Machine information registers
`define CSR_mvendorid       12'hF11     // Vendor ID
`define CSR_marchid         12'hF12     // Architecture ID.
`define CSR_mimpid          12'hF13     // Implementation ID.
`define CSR_mhartid         12'hF14     // Hardware thread ID.
`define CSR_mconfigptr      12'hF15     // Pointer to configuration data structure.

// Machine Trap Setup
`define CSR_mstatus         12'h300     // Machine status register.
`define CSR_misa            12'h301     // ISA and extensions
`define CSR_medeleg         12'h302     // Machine exception delegation register.
`define CSR_mideleg         12'h303     // Machine interrupt delegation register.
`define CSR_mie             12'h304     // Machine interrupt-enable register.
`define CSR_mtvec           12'h305     // Machine trap-handler base address.
`define CSR_mcounteren      12'h306     // Machine counter enable.
`define CSR_mstatush        12'h310     // Additional machine status register, RV32 only.

// Machine Trap Handling
`define CSR_mscratch        12'h340     // Scratch register for machine trap handlers.
`define CSR_mepc            12'h341     // Machine exception program counter.
`define CSR_mcause          12'h342     // Machine trap cause.
`define CSR_mtval           12'h343     // Machine bad address or instruction.
`define CSR_mip             12'h344     // Machine interrupt pending.
`define CSR_mtinst          12'h34A     // Machine trap instruction (transformed).
`define CSR_mtval2          12'h34B     // Machine bad guest physical address.

// Machine Counter/Timers
`define CSR_mcycle          12'hB00     // Machine cycle counter.
`define CSR_minstret        12'hB02     // Machine instructions-retired counter.
`define CSR_mhpmcounter3    12'hB03     // Machine performance-monitoring counter.
`define CSR_mhpmcounter4    12'hB04     // Machine performance-monitoring counter.
`define CSR_mhpmcounter31   12'hB1F     // Machine performance-monitoring counter.
`define CSR_mcycleh         12'hB80     // Upper 32 bits of mcycle, RV32 only.
`define CSR_minstreth       12'hB82     // Upper 32 bits of minstret, RV32 only.
`define CSR_mhpmcounter3h   12'hB83     // Upper 32 bits of mhpmcounter3, RV32 only.
`define CSR_mhpmcounter4h   12'hB84     // Upper 32 bits of mhpmcounter4, RV32 only.
`define CSR_mhpmcounter31h  12'hB9F     // Upper 32 bits of mhpmcounter31, RV32 only.

// Machine Counter Setup
`define CSR_mcountinhibit   12'h320     // Machine counter-inhibit register.
`define CSR_mhpmevent3      12'h323     // Machine performance-monitoring event selector.
`define CSR_mhpmevent4      12'h324     // Machine performance-monitoring event selector.
`define CSR_mhpmevent31     12'h33F     // Machine performance-monitoring event selector.

// Debug/Trace Registers (shared with Debug Mode)
`define CSR_tselect         12'h7A0     // Debug/Trace trigger register select.
`define CSR_tdata1          12'h7A1     // First Debug/Trace trigger data register.
`define CSR_tdata2          12'h7A2     // Second Debug/Trace trigger data register.
`define CSR_tdata3          12'h7A3     // Third Debug/Trace trigger data register.
`define CSR_mcontext        12'h7A8     // Machine-mode context register.

// Debug Mode Registers
`define CSR_dcsr            12'h7B0     // Debug control and status register.
`define CSR_dpc             12'h7B1     // Debug PC.
`define CSR_dscratch0       12'h7B2     // Debug scratch register 0.
`define CSR_dscratch1       12'h7B3     // Debug scratch register 1.

`endif // __CSR_DEFS_VH__
