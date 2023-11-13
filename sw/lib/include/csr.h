#pragma once
#include <utils.h>
#include <stdint.h>

enum CSR_ID
{
    CSR_FFLAGS         = 0x001, /**< 0x001 - fflags (r/w): Floating-point accrued exception flags */
    CSR_FRM            = 0x002, /**< 0x002 - frm    (r/w): Floating-point dynamic rounding mode */
    CSR_FCSR           = 0x003, /**< 0x003 - fcsr   (r/w): Floating-point control/status register (frm + fflags) */

    CSR_MSTATUS        = 0x300, /**< 0x300 - mstatus    (r/w): Machine status register */
    CSR_MISA           = 0x301, /**< 0x301 - misa       (r/-): CPU ISA and extensions (read-only in NEORV32) */
    CSR_MIE            = 0x304, /**< 0x304 - mie        (r/w): Machine interrupt-enable register */
    CSR_MTVEC          = 0x305, /**< 0x305 - mtvec      (r/w): Machine trap-handler base address (for ALL traps) */
    CSR_MCOUNTEREN     = 0x306, /**< 0x305 - mcounteren (r/w): Machine counter enable register (controls access rights from U-mode) */

    CSR_MENVCFG        = 0x30a, /**< 0x30a - menvcfg (r/-): Machine environment configuration register */

    CSR_MSTATUSH       = 0x310, /**< 0x310 - mstatush (r/w): Machine status register - high word */

    CSR_MENVCFGH       = 0x31a, /**< 0x31a - menvcfgh (r/-): Machine environment configuration register - high word */

    CSR_MCOUNTINHIBIT  = 0x320, /**< 0x320 - mcountinhibit (r/w): Machine counter-inhibit register */

    /* Machine hardware performance monitor event selectors */

    CSR_MSCRATCH       = 0x340, /**< 0x340 - mscratch (r/w): Machine scratch register */
    CSR_MEPC           = 0x341, /**< 0x341 - mepc     (r/w): Machine exception program counter */
    CSR_MCAUSE         = 0x342, /**< 0x342 - mcause   (r/w): Machine trap cause */
    CSR_MTVAL          = 0x343, /**< 0x343 - mtval    (r/-): Machine bad address or instruction */
    CSR_MIP            = 0x344, /**< 0x344 - mip      (r/-): Machine interrupt pending register */

    /* Physical memory protection configuration registers */

    /* Physical memory protection address registers */

    CSR_MCYCLE         = 0xb00, /**< 0xb00 - mcycle   (r/w): Machine cycle counter low word */
    CSR_MINSTRET       = 0xb02, /**< 0xb02 - minstret (r/w): Machine instructions-retired counter low word */

    /* Machine hardware performance monitors */

    CSR_MCYCLEH        = 0xb80, /**< 0xb80 - mcycleh   (r/w): Machine cycle counter high word */
    CSR_MINSTRETH      = 0xb82, /**< 0xb82 - minstreth (r/w): Machine instructions-retired counter high word */

    /* Machine hardware performance monitor */

    CSR_CYCLE          = 0xc00, /**< 0xc00 - cycle   (r/-): Cycle counter low word (from MCYCLE) */
    CSR_TIME           = 0xc01, /**< 0xc01 - time    (r/-): Timer low word (from MTIME.TIME_LO) */
    CSR_INSTRET        = 0xc02, /**< 0xc02 - instret (r/-): Instructions-retired counter low word (from MINSTRET) */

    CSR_CYCLEH         = 0xc80, /**< 0xc80 - cycleh   (r/-): Cycle counter high word (from MCYCLEH) */
    CSR_TIMEH          = 0xc81, /**< 0xc81 - timeh    (r/-): Timer high word (from MTIME.TIME_HI) */
    CSR_INSTRETH       = 0xc82, /**< 0xc82 - instreth (r/-): Instructions-retired counter high word (from MINSTRETH) */

    CSR_MVENDORID      = 0xf11, // 0xf11 - mvendorid  (r/-): Vendor ID
    CSR_MARCHID        = 0xf12, // 0xf12 - marchid    (r/-): Architecture ID
    CSR_MIMPID         = 0xf13, // 0xf13 - mimpid     (r/-): Implementation ID/version
    CSR_MHARTID        = 0xf14, // 0xf14 - mhartid    (r/-): Hardware thread ID (always 0)
    CSR_MCONFIGPTR     = 0xf15  // 0xf15 - mconfigptr (r/-): Machine configuration pointer register
};


/**
 * @brief Read CSR Register
 */
inline uint32_t __attribute__ ((always_inline)) CSR_read(const int csr_id)
{
    register uint32_t csr_data;
    asm volatile ("csrr %[result], %[input_i]" : [result] "=r" (csr_data) : [input_i] "i" (csr_id));
    return csr_data;
}


/**
 * @brief Write to a CSR register
 */
inline void __attribute__ ((always_inline)) CSR_write(const int csr_id, uint32_t data)
{
    register uint32_t csr_data = data;
    asm volatile ("csrw %[input_i], %[input_j]" :  : [input_i] "i" (csr_id), [input_j] "r" (csr_data));
}
