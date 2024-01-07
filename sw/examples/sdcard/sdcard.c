#include "platform.h"
#include <stdio.h>
#include <spi.h>
#include <sd.h>


int main(){
    serial_init(UART_BAUD_115200);
    
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
    
    uint8_t rc = sd_init(&cfg);

    printf("read Sector 0\n");
    uint8_t rbuf[512], res[5], token;

    res[0] = sdc_read_block(&cfg, 0x00000000, rbuf, &token);


    // print response
    if(SD_RESP1_NO_ERROR(res[0]) && (token == 0xFE)) {
        dumphexbuf((char*)rbuf, 512, 0x00000000);
    }
    else {
        puts("Error reading sector\n");
    }


    // initialize buffer
    uint8_t wbuf[512];
    for(uint16_t i = 0; i < 512; i++) wbuf[i] = 0x55;

    res[0] = sdc_write_block(&cfg, 0x00000000, wbuf, &token);
    // print response
    if(SD_RESP1_NO_ERROR(res[0]) && (token == 0x05)) {
        puts("Write OK\n");
    }
    else {
        puts("Error writing\n");
    }


    // read again
    res[0] = sdc_read_block(&cfg, 0x00000000, rbuf, &token);
    if(SD_RESP1_NO_ERROR(res[0]) && (token == 0xFE)) {
        dumphexbuf((char*)rbuf, 512, 0x00000000);
    }
    else {
        puts("Error reading sector\n");
    }

    // verify
    for(int i=0; i<512; i++){
        if(rbuf[i] != wbuf[i]){
            printf("TEST FAIL! @ i=%d\n", i);
            return 1;
        }
    }

    puts("TEST PASSED!\n");
    return 0;
}