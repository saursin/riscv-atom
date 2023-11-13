#include <platform.h>
#include <mmio.h>
#include <utils.h>
#include <spi.h>


#define SPI_REG_SCKDIV_OFFSET   0x00
#define SPI_REG_SCTRL_OFFSET    0x04
#define SPI_REG_TDATA_OFFSET    0x08
#define SPI_REG_RDATA_OFFSET    0x0c
#define SPI_REG_CSCTRL_OFFSET   0x10
#define SPI_REG_DCTRL_OFFSET    0x14

#define _spi_busy() (REG32(SPI_ADDR, SPI_REG_SCTRL_OFFSET) >> 31)

int spi_init(struct SPI_Config * cfg) {
    while(_spi_busy()) {
        asm volatile("");
    }
    
    // set divisor reg
    uint32_t fratio = (CLK_FREQ/cfg->baudrate);
    REG32(cfg->base_addr, SPI_REG_SCKDIV_OFFSET) = fratio - 1;

    // set sctrl reg
    uint32_t spi_sctrl = 0;
    spi_sctrl = cfg->enable ? bitset(spi_sctrl, 0): spi_sctrl;
    spi_sctrl = cfg->pol ? bitset(spi_sctrl, 1): spi_sctrl;
    spi_sctrl = cfg->pha ? bitset(spi_sctrl, 2): spi_sctrl;
    spi_sctrl = cfg->lsb_first ? bitset(spi_sctrl, 3): spi_sctrl;
    spi_sctrl = cfg->loopback_enable ? bitset(spi_sctrl, 4): spi_sctrl;
    REG32(cfg->base_addr, SPI_REG_SCTRL_OFFSET) = spi_sctrl;

    uint32_t spi_csctrl = cfg->cs_mode;
    if(cfg->cs_mode == CSMODE_AUTO) {
        spi_csctrl = bitset(spi_csctrl, 24 + cfg->device_num);
    }
    else if (cfg->cs_mode == CSMODE_DISABLE) {
        // In this case device_num is used by select/deselect functions
        spi_csctrl &= 0x00ffffff; // clear active cs
    } else {
        return 1;   // invalid mode
    }
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = spi_csctrl;

    // set dctrl reg
    uint32_t spi_dctrl=0;
    spi_dctrl |= ((uint32_t)cfg->pre_cs_high_delay) << 8;
    spi_dctrl |= ((uint32_t)cfg->post_cs_low_delay);
    REG32(cfg->base_addr, SPI_REG_DCTRL_OFFSET) = spi_dctrl;

    return 0;
}

char spi_transfer(struct SPI_Config * cfg, char b) {
    REG8(cfg->base_addr, SPI_REG_TDATA_OFFSET) = b;
    while(_spi_busy()) {
        asm volatile("");
    }
    return REG8(cfg->base_addr, SPI_REG_RDATA_OFFSET);
}

void spi_select(struct SPI_Config * cfg) {
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitset(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);
}

void spi_deselect(struct SPI_Config * cfg) {
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitclear(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);
}

char *spi_transfer_buf(struct SPI_Config * cfg, char *send_buf, char *recv_buf, unsigned int len)
{
    // Disable hardware control
    uint32_t curr_csctrl = REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET);
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = curr_csctrl | 0b11;

    // Select
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitset(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);

    // transfer
    while(len--) {
        REG8(cfg->base_addr, SPI_REG_TDATA_OFFSET) = *(send_buf++);
        while(_spi_busy())
            asm volatile("");
        *(recv_buf++) = REG8(cfg->base_addr, SPI_REG_RDATA_OFFSET);
    }
    
    // Deselect
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitclear(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);

    // restore previous mode
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = curr_csctrl;
    return recv_buf;
}

char *spi_get_buf(struct SPI_Config * cfg, char send_val, char *recv_buf, unsigned int len)
{
    // Disable hardware control
    uint32_t curr_csctrl = REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET);
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = curr_csctrl | 0b11;

    // Select
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitset(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);

    // transfer
    while(len--) {
        REG8(cfg->base_addr, SPI_REG_TDATA_OFFSET) = send_val;
        while(_spi_busy())
            asm volatile("");
        *(recv_buf++) = REG8(cfg->base_addr, SPI_REG_RDATA_OFFSET);
    }
    
    // Deselect
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = bitclear(REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg->device_num);

    // restore previous mode
    REG32(cfg->base_addr, SPI_REG_CSCTRL_OFFSET) = curr_csctrl;
    return recv_buf;
}