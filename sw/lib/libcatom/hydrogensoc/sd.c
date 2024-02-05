#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <spi.h>

#include <sd.h>

#ifdef SDCARD_SILENT_INIT
    #define SD_DBG(x)
#else
    #define SD_DBG(x) x
#endif

// Configuration
#define SD_NUM_ATTEMPTS     10
#define SD_SPI_INIT_BAUD    200000

//FIXME: change these to time based timeout values
#define SD_MAX_READ_ATTEMPTS    1563
#define SD_MAX_WRITE_ATTEMPTS   3907

// Helper Macros
#define R1_CHECK_CARD_RDY(X)        ((X) == 0)
#define R1_CHECK_MSB_ERROR(X)       ((X) & 0b10000000)
#define R1_CHECK_PARAM_ERROR(X)     ((X) & 0b01000000)
#define R1_CHECK_ADDR_ERROR(X)      ((X) & 0b00100000)
#define R1_CHECK_ERASE_SEQ_ERROR(X) ((X) & 0b00010000)
#define R1_CHECK_CRC_ERROR(X)       ((X) & 0b00001000)
#define R1_CHECK_ILLEGAL_CMD(X)     ((X) & 0b00000100)
#define R1_CHECK_ERASE_RESET(X)     ((X) & 0b00000010)
#define R1_CHECK_IN_IDLE(X)         ((X) & 0b00000001)

#define R7_GET_CMD_VER(X)           ((X >> 4) & 0xF0)
#define R7_GET_VOL_ACC(X)           (X & 0x1F)
#define R7_VOLTAGE_ACC_27_33        0b00000001
#define R7_VOLTAGE_ACC_LOW          0b00000010
#define R7_VOLTAGE_ACC_RES1         0b00000100
#define R7_VOLTAGE_ACC_RES2         0b00001000

#define R3_POWER_UP_STATUS(X)       ((X) & 0x40)
#define R3_CCS_VAL(X)               ((X) & 0x40)
#define R3_VDD_2728(X)              ((X) & 0b10000000)
#define R3_VDD_2829(X)              ((X) & 0b00000001)
#define R3_VDD_2930(X)              ((X) & 0b00000010)
#define R3_VDD_3031(X)              ((X) & 0b00000100)
#define R3_VDD_3132(X)              ((X) & 0b00001000)
#define R3_VDD_3233(X)              ((X) & 0b00010000)
#define R3_VDD_3334(X)              ((X) & 0b00100000)
#define R3_VDD_3435(X)              ((X) & 0b01000000)
#define R3_VDD_3536(X)              ((X) & 0b10000000)


uint8_t sdc_read_res1(struct SPI_Config *cfg);
void sdc_read_res3_or_7(struct SPI_Config *cfg, uint8_t *res);
void sdc_powerup_seq(struct SPI_Config *cfg);

///////////////////////////////////////////////////////////////////////////////
static void __print_buf(uint8_t *buf, unsigned buflen) {
    puts("{ ");
    for(unsigned i=0; i<buflen; i++) {
        printf("0x0%02x%s ", buf[i], (i==buflen-1) ? "" : ",");
    }
    puts("}");
}

void sd_print_r1_resp(uint8_t res) {
    printf("Res1: 0x%02x\n", res);
    if(R1_CHECK_CARD_RDY(res))          puts("- Card Ready\n");
    if(R1_CHECK_MSB_ERROR(res))         puts("- Err MSB=1\n");
    if(R1_CHECK_PARAM_ERROR(res))       puts("- Param Err\n");
    if(R1_CHECK_ADDR_ERROR(res))        puts("- Addr Err\n");
    if(R1_CHECK_ERASE_SEQ_ERROR(res))   puts("- Erase Seq Err\n");
    if(R1_CHECK_CRC_ERROR(res))         puts("- CRC Err\n");
    if(R1_CHECK_ILLEGAL_CMD(res))       puts("- Illegal Cmd\n");
    if(R1_CHECK_ERASE_RESET(res))       puts("- Erase Rst Err\n");
    if(R1_CHECK_IN_IDLE(res))           puts("- In Idle State\n");
}

void sd_print_r7_resp(uint8_t *res) {
    puts("Res7:"); __print_buf(res, 5); putchar('\n');
    
    sd_print_r1_resp(res[0]);
    if(res[0] > 1) return;

    printf("- Cmd Version: 0x%x\n", R7_GET_CMD_VER(res[1]));
    puts("- Voltage Accepted: ");
    switch(R7_GET_VOL_ACC(res[3])){
        case R7_VOLTAGE_ACC_27_33:  puts("2.7-3.6V"); break;
        case R7_VOLTAGE_ACC_LOW:    puts("Low Voltage"); break;
        case R7_VOLTAGE_ACC_RES1:
        case R7_VOLTAGE_ACC_RES2:   puts("Reserved"); break;
        default:                    puts("Not Defined"); break;
    }
    putchar('\n');
    printf("- Check pattern: 0x%x\n", res[4]);
}

void sd_print_r3_resp(uint8_t *res) {
    puts("Res3:"); __print_buf(res, 5); putchar('\n');
    
    sd_print_r1_resp(res[0]);
    if(res[0] > 1) return;

    puts("- Card Power Up Status: ");
    if(R3_POWER_UP_STATUS(res[1])) {
        puts("Ready\n");
        printf("- CCS Status: %c\n", R3_CCS_VAL(res[1]) ? '1' : '0');
    }
    else {
        puts("Busy\n");
    }

    puts("- VDD Window: ");
    if(R3_VDD_2728(res[3])) puts("2.7-2.8, ");
    if(R3_VDD_2829(res[2])) puts("2.8-2.9, ");
    if(R3_VDD_2930(res[2])) puts("2.9-3.0, ");
    if(R3_VDD_3031(res[2])) puts("3.0-3.1, ");
    if(R3_VDD_3132(res[2])) puts("3.1-3.2, ");
    if(R3_VDD_3233(res[2])) puts("3.2-3.3, ");
    if(R3_VDD_3334(res[2])) puts("3.3-3.4, ");
    if(R3_VDD_3435(res[2])) puts("3.4-3.5, ");
    if(R3_VDD_3536(res[2])) puts("3.5-3.6");
    puts("\n");
}

enum SDRespType {
    RESP1,
    RESP3,
    RESP7
};

typedef struct {
    uint8_t cmd;
    uint32_t arg;
    uint8_t crc;
    enum SDRespType resp_type;
} SDCmd_attr;

typedef enum {
    SD_CMD0  = 0,
    SD_CMD8  = 1,
    SD_CMD17 = 2,
    SD_CMD24 = 3,
    SD_CMD41 = 4, 
    SD_CMD55 = 5, 
    SD_CMD58 = 6
} SDCmd;

const SDCmd_attr sdcmds[7] = {
    /* SD_CMD0  */ {.cmd=0,  .arg=0x00000000, .crc=0x94, .resp_type=RESP1},
    /* SD_CMD8  */ {.cmd=8,  .arg=0x000001AA, .crc=0x86, .resp_type=RESP7},
    /* SD_CMD17 */ {.cmd=17, .arg=0x00000000, .crc=0x00, .resp_type=RESP1},
    /* SD_CMD24 */ {.cmd=24, .arg=0x00000000, .crc=0x00, .resp_type=RESP1},
    /* SD_CMD41 */ {.cmd=41, .arg=0x40000000, .crc=0x00, .resp_type=RESP1},
    /* SD_CMD55 */ {.cmd=55, .arg=0x00000000, .crc=0x00, .resp_type=RESP1},
    /* SD_CMD58 */ {.cmd=58, .arg=0x00000000, .crc=0x00, .resp_type=RESP3}
};

#define _spi_select(cfg) \
    spi_transfer(cfg, 0xFF); \
    spi_select(cfg); \
    spi_transfer(cfg, 0xFF); 

#define _spi_deselect(cfg) \
    spi_transfer(cfg, 0xFF); \
    spi_deselect(cfg); \
    spi_transfer(cfg, 0xFF); \

void sdc_cmd(struct SPI_Config *cfg, SDCmd cmd, uint8_t * resp) {
    // send command
    spi_transfer(cfg, sdcmds[cmd].cmd | 0x40);

    // send arg
    spi_transfer(cfg, sdcmds[cmd].arg >> 24);
	spi_transfer(cfg, sdcmds[cmd].arg >> 16);
	spi_transfer(cfg, sdcmds[cmd].arg >> 8);
    spi_transfer(cfg, sdcmds[cmd].arg);
    
    // send CRC
    spi_transfer(cfg, sdcmds[cmd].crc | 0x1);

    // read response
    switch(sdcmds[cmd].resp_type) {
        case RESP1: 
            resp[0] = sdc_read_res1(cfg); 
            break;

        case RESP3:
        case RESP7:
            sdc_read_res3_or_7(cfg, resp); 
            break;
        
        default:
            resp[0] = sdc_read_res1(cfg); 
            break;
    }
}

uint8_t sdc_read_res1(struct SPI_Config *cfg) {
    unsigned ntry = 8;
    uint8_t resp;
    do {
        resp = spi_transfer(cfg, 0xFF);
    } while((resp == 0xff) & (ntry-- > 0));
    return resp;
}

void sdc_read_res3_or_7(struct SPI_Config *cfg, uint8_t *res) {
    // read R1
    res[0] = sdc_read_res1(cfg);

    // if error reading R1, return
    if(res[0] > 1) return;

    // read remaining bytes
    res[1] = spi_transfer(cfg, 0x0);
    res[2] = spi_transfer(cfg, 0x0);
    res[3] = spi_transfer(cfg, 0x0);
    res[4] = spi_transfer(cfg, 0x0);
}

void sdc_powerup_seq(struct SPI_Config *cfg) {
    // deselect SPI
    spi_deselect(cfg);
    
    // wait for some time
    sleep_ms(1);
    
    // send 80 clock cycles to synchronize
    for(int i=0; i<80; i+=8)
        spi_transfer(cfg, 0xff);
}

uint8_t sd_init(struct SPI_Config *cfg)
{
    cfg->cs_mode = CSMODE_DISABLE;
    cfg->baudrate = SD_SPI_INIT_BAUD;
    spi_init(cfg);

    uint8_t res[5];
    uint8_t ntries = 0;

    // Step - 1: Power UP Sequence
    SD_DBG(puts("SDC: Powering up\n");)
    sdc_powerup_seq(cfg);


    // Step - 2: goto IDLE
    SD_DBG(puts("SDC: Sending CMD0..\n");)
    ntries = SD_NUM_ATTEMPTS;
    do {
        _spi_select(cfg)
        sdc_cmd(cfg, SD_CMD0, res);
        _spi_deselect(cfg)
    } while ((res[0] != 0x01) && (ntries-- > 0));

    if(ntries==0)   
        return SD_ERR_UNKNOWN_CARD;


    // Step - 3: Send interface conditions
    SD_DBG(puts("SDC: Sending CMD8..\n");)
    _spi_select(cfg)
    sdc_cmd(cfg, SD_CMD8, res);
    _spi_deselect(cfg)
    SD_DBG(sd_print_r7_resp(res);)

    // check idle
    if(res[0] != 0x01)
        return SD_ERR_MAYBE_VER1CARD;
    // check echo pattern
    if(res[4] != 0xAA)
        return SD_ERR_UNKNOWN_CARD;


    // Step - 4: Initialization
    SD_DBG(puts("SDC: Initializing..\n");)
    ntries = 0;
    do {
        if(ntries > SD_NUM_ATTEMPTS) {
            return SD_ERR_UNKNOWN_CARD;
        }

        // send app cmd
        SD_DBG(printf("SDC: Sending CMD55... (Try:%d)\n", ntries);)
        _spi_select(cfg)
        sdc_cmd(cfg, SD_CMD55, res);
        _spi_deselect(cfg)
        SD_DBG(sd_print_r1_resp(res[0]);)

        // if no error in response
        if(res[0] < 2)
        {
            SD_DBG(puts("SDC: Sending ACMD41...\n");)
            _spi_select(cfg)
            sdc_cmd(cfg, SD_CMD41, res);
            _spi_deselect(cfg)
            SD_DBG(sd_print_r1_resp(res[0]);)
        }

        // wait
        sleep_ms(10);
        ntries++;
    } while(!R1_CHECK_CARD_RDY(res[0]));

    // Step - 5: read OCR
    SD_DBG(puts("SDC: Send CMD58 (Read OCR)..\n");)
    _spi_select(cfg)
    sdc_cmd(cfg, SD_CMD58, res);
    _spi_deselect(cfg)
    SD_DBG(sd_print_r3_resp(res);)

    // check card is ready
    SD_DBG(puts("SDC: Initialization ");)

    if(!(res[1] & 0x80)) {
        SD_DBG(puts("FAILED\n");)
        return SD_ERR_UNKNOWN_CARD;
    }

    SD_DBG(puts("SUCCESS\n");)
    return SD_SUCCESS;
}

uint8_t sdc_read_block(struct SPI_Config *cfg, uint32_t addr, uint8_t *buf, uint8_t *token) {
    uint8_t res1, read;
    uint16_t readAttempts;

    // set token to none
    *token = 0xFF;

    _spi_select(cfg);

    // send command 17
    sdc_cmd(cfg, SD_CMD17, &res1);

    // if response received from card
    if(res1 != 0xFF)
    {
        // wait for a response token (timeout = 100ms)
        readAttempts = 0;
        while(++readAttempts != SD_MAX_READ_ATTEMPTS)
            if((read = spi_transfer(cfg, 0xFF)) != 0xFF) break;

        // if response token is 0xFE
        if(read == SD_START_TOKEN)
        {
            // read 512 byte block
            for(uint16_t i = 0; i < SD_BLOCK_LEN; i++) 
                *buf++ = spi_transfer(cfg, 0xFF);

            // read (& discard) 16-bit CRC
            spi_transfer(cfg, 0xFF);
            spi_transfer(cfg, 0xFF);
        }

        // set token to card response
        *token = read;
    }

    // deassert chip select
    _spi_deselect(cfg);
    return res1;
}

uint8_t sdc_write_block(struct SPI_Config *cfg, uint32_t addr, uint8_t *buf, uint8_t *token)
{
    uint8_t res1, readAttempts, read;

    // set token to none
    *token = 0xFF;

    // assert chip select
    _spi_select(cfg)

    // send CMD24
    sdc_cmd(cfg, SD_CMD24, &res1);

    // if no error
    if(R1_CHECK_CARD_RDY(res1))
    {
        // send start token
        spi_transfer(cfg, SD_START_TOKEN);

        // write buffer to card
        for(uint16_t i = 0; i < SD_BLOCK_LEN; i++) 
            spi_transfer(cfg, buf[i]);

        // wait for a response (timeout = 250ms)
        readAttempts = 0;
        while(++readAttempts != SD_MAX_WRITE_ATTEMPTS)
            if((read = spi_transfer(cfg, 0xFF)) != 0xFF) { *token = 0xFF; break; }

        // if data accepted
        if((read & 0x1F) == 0x05)
        {
            // set token to data accepted
            *token = 0x05;

            // wait for write to finish (timeout = 250ms)
            readAttempts = 0;
            while(spi_transfer(cfg, 0xFF) == 0x00) {
                if(++readAttempts == SD_MAX_WRITE_ATTEMPTS) { 
                    *token = 0x00; 
                    break; 
                }
            }
        }
    }

    // deassert chip select
    _spi_deselect(cfg)
    return res1;
}
