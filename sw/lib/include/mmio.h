#ifndef __MMIO_H__
#define __MMIO_H__

// Memory-Mapped Reg Access Macros
#define REG8(base, offset) *((volatile uint8_t*)    (base + offset))
#define REG16(base, offset) *((volatile uint16_t*)  (base + offset))
#define REG32(base, offset) *((volatile uint32_t*)  (base + offset))

#endif // __MMIO_H__