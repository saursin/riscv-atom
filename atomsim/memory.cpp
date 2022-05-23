#include "memory.hpp"
#include "except.hpp"

#include <stdint.h>
#include <iostream>

Memory::Memory(uint32_t num_bytes, uint32_t address_offset, bool is_write_protected):
    size_(num_bytes),
    addr_offset_(address_offset),
    is_write_protected_(is_write_protected)
{
    // Allocate memory
    if(!(mem_ = new uint8_t[num_bytes]))
        throw Atomsim_exception("Memory allocation failed");
}


Memory::~Memory()
{
    // deallocate memory
    delete [] mem_;
    size_ = 0;
}


void Memory::fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)
{
    // exception message buffer
    char except_buf[50];

    uint32_t rel_start_addr = start_addr - addr_offset_;    // relative to current memory block

    for (uint32_t i = 0; i < buf_sz; i++)
    {
        // get index
        uint32_t indx = rel_start_addr + i;
        
        if (!(indx < size_))    // check bounds
        {
            sprintf(except_buf, "invalid fetch address [0x%08x]", start_addr + i);
            throw Atomsim_exception(except_buf);
        }

        // copy from mem
        buf[i] = mem_[indx];
    }
}


void Memory::store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)
{
    if(is_write_protected_)
        throw "attempted to store in a write-protected memory";

    // exception message buffer
    char except_buf[50];

    uint32_t rel_start_addr = start_addr - addr_offset_;    // relative to current memory block

    for (uint32_t i = 0; i < buf_sz; i++)
    {
        // get index
        uint32_t indx = rel_start_addr + i;
        
        if (!(indx < size_))    // check bounds
        {
            sprintf(except_buf, "invalid store address [0x%08x]", start_addr + i);
            throw Atomsim_exception(except_buf);
        }

        // copy to mem
        mem_[indx] = buf[i];
    }
}