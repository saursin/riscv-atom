#pragma once

#include "common.h"

#include <stdint.h>
#include <spi.h>
#include <utils.h>
#include <mmio.h>

// Flash definitions
#define FLASH_CMD_READ          0x03
#define FLASH_CMD_READID        0X9f
#define SPI_REG_SCKDIV_OFFSET   0x00
#define SPI_REG_SCTRL_OFFSET    0x04
#define SPI_REG_TDATA_OFFSET    0x08
#define SPI_REG_RDATA_OFFSET    0x0c
#define SPI_REG_CSCTRL_OFFSET   0x10
#define SPI_REG_DCTRL_OFFSET    0x14

char flashgetid(struct SPI_Config *cfg){
    // select
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitset(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);
    spi_transfer(cfg, FLASH_CMD_READID);
    char id = spi_transfer(cfg, 0x0);
    // deselect
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitclear(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);
    return id;
}

int flashcpy(struct SPI_Config *cfg, uint8_t *dest, uint32_t src_addr, uint32_t len) {
    // select
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitset(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);
    spi_transfer(cfg, FLASH_CMD_READ);
    spi_transfer(cfg, src_addr >> 16);
	spi_transfer(cfg, src_addr >> 8);
	spi_transfer(cfg, src_addr);

    while(len--) {
        REG8(cfg->base_addr, SPI_REG_TDATA_OFFSET) = 0x0;
        while(REG32(SPI_ADDR, SPI_REG_SCTRL_OFFSET) >> 31)
            asm volatile("");
        *(dest++) = REG8(cfg->base_addr, SPI_REG_RDATA_OFFSET);
    }

    // deselect
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitclear(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);
    return 0;
}

void flashboot(uint8_t *dest, uint32_t src_addr, uint32_t len) {
    // Init
    struct SPI_Config cfg = {
        cfg.base_addr = SPI_ADDR,
        cfg.enable = true,
        cfg.pha = false,
        cfg.pol = false,
        cfg.lsb_first = false,
        cfg.baudrate = 1000000,
        cfg.device_num = 0,
        cfg.cs_mode = CSMODE_DISABLE,
        cfg.post_cs_low_delay = 1,
        cfg.pre_cs_high_delay = 1,
        cfg.loopback_enable=false
    };
    spi_init(&cfg);

    // Check ID
    uint8_t manufacturer_id = flashgetid(&cfg);
    P(puts("flashboot: manuf id = 0x");)
    P(puthex(manufacturer_id);)
    P(putchar('\n');)
    if(manufacturer_id == 0x00 || manufacturer_id == 0xff){
        boot_panic(RCODE_FLASHBOOT_INVALID_DEVICE);
    }

    // Copy
    P(puts("\nflashboot: copying from " EXPAND_AND_STRINGIFY(FLASH_IMG_OFFSET));)
    flashcpy(&cfg, dest, src_addr, len);
    P(puts(" - OK\n");)

    // Deinit
    cfg.cs_mode = CSMODE_AUTO;
    spi_init(&cfg);
}
