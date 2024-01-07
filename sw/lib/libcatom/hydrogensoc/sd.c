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

// Commands
#define SD_CMD0         0
#define SD_CMD0_ARG     0x00000000
#define SD_CMD0_CRC     0x94
#define SD_DUMMY_BYTE   0xff

#define SD_CMD8         8
#define SD_CMD8_ARG     0x0000001AA
#define SD_CMD8_CRC     0x86 //(1000011 << 1)

#define SD_CMD58        58
#define SD_CMD58_ARG    0x00000000
#define SD_CMD58_CRC    0x00

#define SD_CMD55        55
#define SD_CMD55_ARG    0x00000000
#define SD_CMD55_CRC    0x00

#define SD_ACMD41       41
#define SD_ACMD41_ARG   0x40000000
#define SD_ACMD41_CRC   0x00

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

void sdc_cmd(struct SPI_Config *cfg, uint8_t cmd, uint32_t arg, uint8_t crc) {
    spi_transfer(cfg, cmd | 0x40);

    spi_transfer(cfg, arg >> 24);
	spi_transfer(cfg, arg >> 16);
	spi_transfer(cfg, arg >> 8);
    spi_transfer(cfg, arg);
    
    spi_transfer(cfg, crc | 0x1);
}

uint8_t sdc_send_cmd0(struct SPI_Config *cfg) {
    // assert chip select
    spi_transfer(cfg, 0xFF);
    spi_select(cfg);
    spi_transfer(cfg, 0xFF);

    // send CMD0
    sdc_cmd(cfg, SD_CMD0, SD_CMD0_ARG, SD_CMD0_CRC);

    // read response
    uint8_t res1 = sdc_read_res1(cfg);

    // deassert chip select
    spi_transfer(cfg, 0xFF);
    spi_deselect(cfg);
    spi_transfer(cfg, 0xFF);

    return res1;
}

void sdc_send_cmd8(struct SPI_Config *cfg, uint8_t *res) {
    // IFC Cond Command
    // assert chip select
    spi_transfer(cfg, 0xFF);
    spi_select(cfg);
    spi_transfer(cfg, 0xFF);

    // send CMD8
    sdc_cmd(cfg, SD_CMD8, SD_CMD8_ARG, SD_CMD8_CRC);

    // read response 3
    sdc_read_res3_or_7(cfg, res);

    // deassert chip select
    spi_transfer(cfg, 0xFF);
    spi_deselect(cfg);
    spi_transfer(cfg, 0xFF);
}

uint8_t sd_send_cmd55(struct SPI_Config *cfg) {
    // APP CMD
    // assert chip select
    spi_transfer(cfg, 0xFF);
    spi_select(cfg);
    spi_transfer(cfg, 0xFF);

    // send CMD55
    sdc_cmd(cfg, SD_CMD55, SD_CMD55_ARG, SD_CMD55_CRC);

    // read response
    uint8_t res1 = sdc_read_res1(cfg);

    // deassert chip select
    spi_transfer(cfg, 0xFF);
    spi_deselect(cfg);
    spi_transfer(cfg, 0xFF);

    return res1;
}

uint8_t sd_send_acmd41(struct SPI_Config *cfg)
{
    // OP Cond CMD
    // assert chip select
    spi_transfer(cfg, 0xFF);
    spi_select(cfg);
    spi_transfer(cfg, 0xFF);

    // send CMD0
    sdc_cmd(cfg, SD_ACMD41, SD_ACMD41_ARG, SD_ACMD41_CRC);

    // read response
    uint8_t res1 = sdc_read_res1(cfg);

    // deassert chip select
    spi_transfer(cfg, 0xFF);
    spi_deselect(cfg);
    spi_transfer(cfg, 0xFF);

    return res1;
}

void sdc_send_cmd58(struct SPI_Config *cfg, uint8_t *res) {
    // assert chip select
    spi_transfer(cfg, 0xFF);
    spi_select(cfg);
    spi_transfer(cfg, 0xFF);

    // send CMD58
    sdc_cmd(cfg, SD_CMD58, SD_CMD58_ARG, SD_CMD58_CRC);

    // read response
    sdc_read_res3_or_7(cfg, res);

    // deassert chip select
    spi_transfer(cfg, 0xFF);
    spi_deselect(cfg);
    spi_transfer(cfg, 0xFF);
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
    uint8_t cmdAttempts = 0;


    // Step - 1: Power UP Sequence
    SD_DBG(puts("SDC: Powering up\n");)
    sdc_powerup_seq(cfg);


    // Step - 2: goto IDLE
    SD_DBG(puts("SDC: Sending CMD0..\n");)
    while((res[0] = sdc_send_cmd0(cfg)) != 0x01) {
        cmdAttempts++;
        if(cmdAttempts > SD_NUM_ATTEMPTS) 
            return SD_ERR_UNKNOWN_CARD;
    }
    SD_DBG(sd_print_r1_resp(res[0]);)


    // Step - 3: Send interface conditions
    SD_DBG(puts("SDC: Sending CMD8..\n");)
    sdc_send_cmd8(cfg, res);
    SD_DBG(sd_print_r7_resp(res);)

    // check idle
    if(res[0] != 0x01)
        return SD_ERR_MAYBE_VER1CARD;
    // check echo pattern
    if(res[4] != 0xAA)
        return SD_ERR_UNKNOWN_CARD;

    // Step - 4: Initialization
    SD_DBG(puts("SDC: Initializing..\n");)
    cmdAttempts = 0;
    do {
        if(cmdAttempts > SD_NUM_ATTEMPTS) {
            return SD_ERR_UNKNOWN_CARD;
        }

        // send app cmd
        SD_DBG(printf("SDC: Sending CMD55... (Try:%d)\n", cmdAttempts);)
        res[0] = sd_send_cmd55(cfg);
        SD_DBG(sd_print_r1_resp(res[0]);)

        // if no error in response
        if(res[0] < 2)
        {
            SD_DBG(puts("SDC: Sending ACMD41...\n");)
            res[0] = sd_send_acmd41(cfg);
            SD_DBG(sd_print_r1_resp(res[0]);)
        }

        // wait
        sleep_ms(10);
        cmdAttempts++;
    } while(!R1_CHECK_CARD_RDY(res[0]));

    // Step - 5: read OCR
    SD_DBG(puts("SDC: Send CMD58 (Read OCR)..\n");)
    sdc_send_cmd58(cfg, res);
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
