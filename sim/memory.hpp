#pragma once

#include <stdint.h>
#include <string>
#include <vector>

/**
 * @brief Union representing a word. 
 * it can be accessed as a word, 2 element array of halfwords or 
 * a 4 element array of bytes.
 */
union Word_alias
{
    uint8_t byte [4];
    uint16_t hword [2];
    uint32_t word;
};


/**
 * @brief Memory class
 * This class is used to emulate the memories in simulation backend
 */
class Memory
{
public:
	/**
	 * @brief Construct a new Memory object
	 * 
	 * @param num_bytes size of memory
     * @param address_offset address offset for memory device
     * @param is_write_protected write protect flag
	 */
	Memory(uint32_t num_bytes, uint32_t address_offset, bool is_write_protected);


	/**
	 * @brief Destroy the Memory object
	 */
	~Memory();


    /**
     * @brief check if memory is write protected
     * @return true 
     * @return false 
     */
    bool is_write_protected()           {return is_write_protected_;}


    /**
     * @brief Modify write protect permission
     * @param protect
     */
    void set_write_protect(bool protect)    {is_write_protected_ = protect;}


    /**
     * @brief fetch bytes
     * 
     * @param start_addr starting address
     * @param buf byte buffer 
     * @param buf_sz buffer size
     * @throws char* invalid address exception
     */
    void fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz);


    /**
     * @brief store bytes
     * 
     * @param start_addr starting address
     * @param buf byte buffer 
     * @param buf_sz buffer size
     * @throws char* invalid address exception
     */
    void store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz);


    /**
     * @brief Get memory size
     * @return uint32_t size
     */
    uint32_t get_size() { return size_; }


    /**
     * @brief Get the base addr of memory block
     * 
     * @return uint32_t 
     */
    uint32_t get_base_addr() { return addr_offset_;}


    /**
     * @brief check if given address is in range of current memory block
     * 
     * @param addr address
     * @return true 
     * @return false 
     */
    inline bool addr_in_range(uint32_t addr)
    {
        return (addr >= addr_offset_ && addr < (addr_offset_ + size_));
    }

    /**
     * @brief check if given block is in range of current memory block
     * 
     * @param blk_addr block base address
     * @param blk_sz block size
     * @return true 
     * @return false 
     */
    inline bool block_in_range(uint32_t blk_addr, uint32_t blk_sz)
    {
        return addr_in_range(blk_addr)                          // start addr in range
        && ((blk_addr + blk_sz - 1) < (addr_offset_ + size_));  // end address in range
    }

private:
	/**
	 * @brief pointer to memory array
	 */
	uint8_t * mem_;

	/**
	 * @brief size of memory
	 */
	uint32_t size_;

    /**
	 * @brief address offset
	 */
	uint32_t addr_offset_;

    /**
     * @brief write protect flag
     */
    bool is_write_protected_;
};



/**
 * @brief Initialize a memory from an elf file
 * 
 * @param m memory ptr
 * @param filepath elf file path
 * @param flag_signatures section signatures to load
 * @return unsigned entry point
 */
unsigned init_from_elf(Memory * m, std::string filepath, std::vector<int> flag_signatures);