#pragma once

#include <stdint.h>
#include <spi.h>
#include <utils.h>

// Flash definitions
#define FLASH_CMD_READ 0x03
#define SPI_REG_SCKDIV_OFFSET   0x00
#define SPI_REG_SCTRL_OFFSET    0x04
#define SPI_REG_TDATA_OFFSET    0x08
#define SPI_REG_RDATA_OFFSET    0x0c
#define SPI_REG_CSCTRL_OFFSET   0x10
#define SPI_REG_DCTRL_OFFSET    0x14

/**
 * @brief Copies Data from FLAST to dest
 * 
 * @param dest dest buffer
 * @param src_addr source address in flash
 * @param len 
 * @return int 
 */
int flashcpy(uint8_t *dest, uint32_t src_addr, uint32_t len) {
    struct SPI_Config cfg = {
        .base_addr = SPI_ADDR,
        .enable = true,
        .pha = false,
        .pol = false,
        .lsb_first = false,
        .baudrate = 1000000,
        .device_num = 0,
        .cs_mode = CSMODE_DISABLE,
        .post_cs_low_delay = 1,
        .pre_cs_high_delay = 1,
        .loopback_enable=false
    };
    spi_init(&cfg);

    // select
    REG32(cfg.base_addr, SPI_REG_CSCTRL_OFFSET) = bitset(REG32(cfg.base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg.device_num);
    spi_transfer(&cfg, FLASH_CMD_READ);
    spi_transfer(&cfg, src_addr >> 16);
	spi_transfer(&cfg, src_addr >> 8);
	spi_transfer(&cfg, src_addr);

    while(len--) {
        REG8(cfg.base_addr, SPI_REG_TDATA_OFFSET) = 0x0;
        while(REG32(SPI_ADDR, SPI_REG_SCTRL_OFFSET) >> 31)
            asm volatile("");
        *(dest++) = REG8(cfg.base_addr, SPI_REG_RDATA_OFFSET);
    }

    // deselect
    REG32(cfg.base_addr, SPI_REG_CSCTRL_OFFSET) = bitclear(REG32(cfg.base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg.device_num);
    cfg.cs_mode = CSMODE_AUTO;
    spi_init(&cfg);
    return 0;
}