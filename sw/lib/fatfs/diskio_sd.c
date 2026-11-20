// FatFs disk interface: drive 0 is an SD card on SPI chip select 0 (libcatom SD driver)
#include <platform.h>
#include <spi.h>
#include <sd.h>
#include "ff.h"
#include "diskio.h"

#define SD_FAST_BAUD    5000000     // after initialization (cards allow up to 25 MHz)

static struct SPI_Config cfg = {
    .base_addr = SOC_SPI_ADDR,
    .enable = true,
    .pha = false,
    .pol = false,
    .lsb_first = false,
    .baudrate = 1000000,
    .device_num = 0,
    .cs_mode = CSMODE_DISABLE,
    .post_cs_low_delay = 1,
    .pre_cs_high_delay = 1,
    .loopback_enable = false
};

static DSTATUS status = STA_NOINIT;


DSTATUS disk_status(BYTE pdrv) {
    return pdrv ? STA_NOINIT : status;
}


DSTATUS disk_initialize(BYTE pdrv) {
    if(pdrv)
        return STA_NOINIT;
    if(sd_init(&cfg) == SD_SUCCESS) {
        cfg.baudrate = SD_FAST_BAUD;
        spi_init(&cfg);
        status = 0;
    } else {
        status = STA_NOINIT;
    }
    return status;
}


DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    if(pdrv || status)
        return RES_NOTRDY;
    for(; count--; sector++, buff += SD_BLOCK_LEN) {
        uint8_t token;
        if(!SD_RESP1_NO_ERROR(sdc_read_block(&cfg, sector, buff, &token)) || token != SD_START_TOKEN)
            return RES_ERROR;
    }
    return RES_OK;
}


#if !FF_FS_READONLY
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    if(pdrv || status)
        return RES_NOTRDY;
    for(; count--; sector++, buff += SD_BLOCK_LEN) {
        uint8_t token;
        if(!SD_RESP1_NO_ERROR(sdc_write_block(&cfg, sector, (uint8_t *)buff, &token)) || token != SD_DATA_ACCEPTED)
            return RES_ERROR;
    }
    return RES_OK;
}
#endif


DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    if(pdrv || status)
        return RES_NOTRDY;
    if(cmd == CTRL_SYNC)        // writes complete before disk_write returns
        return RES_OK;
    return RES_PARERR;          // no card size: f_mkfs is for the host tool
}
