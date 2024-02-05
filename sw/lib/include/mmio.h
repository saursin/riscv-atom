#pragma once

// Memory-Mapped Reg Access Macros
#define REG8(base, offset) *((volatile uint8_t*)    (base + offset))
#define REG16(base, offset) *((volatile uint16_t*)  (base + offset))
#define REG32(base, offset) *((volatile uint32_t*)  (base + offset))
#define REG64(base, offset) *((volatile uint64_t*)  (base + offset))
