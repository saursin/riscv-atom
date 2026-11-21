// SD card test: write a pattern to one block, read it back and check it, then write the block's
// original contents back, so the card is left as it was.
#include "platform.h"
#include <stdio.h>
#include <spi.h>
#include <sd.h>

// Block 1: on cards formatted by a PC it lies in the unused gap before the first partition
#define TEST_BLOCK  1

static int read_block(struct SPI_Config *cfg, uint32_t block, uint8_t *buf) {
    uint8_t token;
    return SD_RESP1_NO_ERROR(sdc_read_block(cfg, block, buf, &token)) && token == SD_START_TOKEN;
}

static int write_block(struct SPI_Config *cfg, uint32_t block, uint8_t *buf) {
    uint8_t token;
    return SD_RESP1_NO_ERROR(sdc_write_block(cfg, block, buf, &token)) && token == SD_DATA_ACCEPTED;
}

static int same(uint8_t *a, uint8_t *b) {
    for(int i = 0; i < SD_BLOCK_LEN; i++)
        if(a[i] != b[i])
            return 0;
    return 1;
}

int main() {
    struct SPI_Config cfg = {
        cfg.base_addr = SOC_SPI_ADDR,
        cfg.enable = true,
        cfg.pha = false,
        cfg.pol = false,
        cfg.lsb_first = false,
        cfg.baudrate = 1000000,
        cfg.device_num = SD_SPI_DEVICE,
        cfg.cs_mode = CSMODE_DISABLE,
        cfg.post_cs_low_delay = 1,
        cfg.pre_cs_high_delay = 1,
        cfg.loopback_enable=false
    };

    if(sd_init(&cfg) != SD_SUCCESS) {
        puts("no SD card\n");
        return 1;
    }

    uint8_t orig[SD_BLOCK_LEN], pattern[SD_BLOCK_LEN], buf[SD_BLOCK_LEN];
    printf("block %d:\n", TEST_BLOCK);
    if(!read_block(&cfg, TEST_BLOCK, orig)) {
        puts("read failed\n");
        return 1;
    }
    dumphexbuf((char *)orig, SD_BLOCK_LEN, TEST_BLOCK * SD_BLOCK_LEN);

    for(int i = 0; i < SD_BLOCK_LEN; i++)
        pattern[i] = i ^ 0x55;
    int ok = write_block(&cfg, TEST_BLOCK, pattern) && read_block(&cfg, TEST_BLOCK, buf) && same(buf, pattern);
    puts(ok ? "write and read back: OK\n" : "write and read back: FAILED\n");

    // put the original contents back, whatever happened above
    int restored = write_block(&cfg, TEST_BLOCK, orig) && read_block(&cfg, TEST_BLOCK, buf) && same(buf, orig);
    puts(restored ? "original contents restored\n" : "RESTORE FAILED: block changed\n");

    if(!(ok && restored))
        return 1;
    puts("TEST PASSED!\n");
    return 0;
}
