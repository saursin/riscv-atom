#ifndef __W25Q64_H__
#define __W25Q64_H__

// CODES
#define CMD_WRITE_ENABLE      0x06
#define CMD_WRITE_DISABLE     0x04
#define CMD_READ_STATUS_R1    0x05
#define CMD_READ_STATUS_R2    0x35
#define CMD_WRITE_STATUS_R    0x01
#define CMD_PAGE_PROGRAM      0x02
#define CMD_QUAD_PAGE_PROGRAM 0x32
#define CMD_BLOCK_ERASE64KB   0xd8
#define CMD_BLOCK_ERASE32KB   0x52
#define CMD_SECTOR_ERASE      0x20
#define CMD_CHIP_ERASE        0xC7
#define CMD_ERASE_SUPPEND     0x75
#define CMD_ERASE_RESUME      0x7A
#define CMD_POWER_DOWN        0xB9
#define CMD_HIGH_PERFORM_MODE 0xA3
#define CMD_CNT_READ_MODE_RST 0xFF
#define CMD_RELEASE_PDOWN_ID  0xAB
#define CMD_MANUFACURER_ID    0x90
#define CMD_READ_UNIQUE_ID    0x4B
#define CMD_JEDEC_ID          0x9f

#define CMD_READ_DATA         0x03
#define CMD_FAST_READ         0x0B
#define CMD_READ_DUAL_OUTPUT  0x3B
#define CMD_READ_DUAL_IO      0xBB
#define CMD_READ_QUAD_OUTPUT  0x6B
#define CMD_READ_QUAD_IO      0xEB
#define CMD_WORD_READ         0xE3

#define SR1_BUSY_MASK	0x01
#define SR1_WEN_MASK	0x02


/**
 * @brief Initialize communication with the flash chip
 */
void W25Q64_init();


/**
 * @brief Write Enable
 */
void W25Q64_writeEnable();


/**
 * @brief Write Disable
 */
void W25Q64_writeDisable();


/**
 * @brief Read status register 1
 * 
 * @return uint8_t 
 */
uint8_t W25Q64_readStatusReg1();


/**
 * @brief Read status register 2
 * 
 * @return uint8_t 
 */
uint8_t W25Q64_readStatusReg2();


/**
 * @brief Check if flash is busy
 * 
 * @return true if busy
 * @return false otherwise
 */
bool W25Q64_isBusy();


/**
 * @brief Program a memory page from 1 byte to 256 bytes(a page) of data.
 * 
 * @param sect_no sector number
 * @param inaddr offset
 * @param buf byte buffer
 * @param n size of buffer
 * @param flagwait set this true if need to wait for completion of operation on chip.
 * @return uint16_t 
 */
uint16_t W25Q64_pageProgram(uint16_t sect_no, uint16_t inaddr, uint8_t * buf, uint16_t n, bool flagwait);


/**
 * @brief Sets all memory withing a sector(4k Bytes) to erased state (0xFF).
 * 
 * @param sect_no sector number
 * @param flagwait set this to true if needed to wait for the completion of operation on the chip.
 */
void W25Q64_eraseSector(uint16_t sect_no, bool flagwait);


/**
 * @brief Erase a 32kB block of data
 * 
 * @param blk_no bock number
 * @param flagwait set this to true if needed to wait for the completion of operation on the chip.
 */
bool W25Q64_erase32Block(uint16_t blk_no, bool flgwait);


/**
 * @brief Erase a 64kB block of data
 * 
 * @param blk_no bock number
 * @param flagwait set this to true if needed to wait for the completion of operation on the chip.
 */
bool W25Q64_erase64Block(uint16_t blk_no, bool flagwait);


/**
 * @brief Erase all data
 *
 */
bool  W25Q64_eraseAll(bool flagwait);


/**
 * @brief Read bytes from flash chip
 * 
 * @param buf buffer to store bytes
 * @param len bytes to read
 * @param addr address to start reading from
 * @return char* buffer to store bytes
 */
uint8_t *W25Q64_read(uint8_t *buf, uint32_t len, uint32_t addr);


#endif //__W25Q64_H__
