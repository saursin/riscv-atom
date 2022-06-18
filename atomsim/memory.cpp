#include "memory.hpp"
#include "except.hpp"

#include "elfio/elfio.hpp"
#include "util.hpp"

#include <stdint.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>


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
        throw Atomsim_exception("attempted to store in a write-protected memory");

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


unsigned init_from_elf(Memory * m, std::string filepath, std::vector<int> flag_signatures)
{
    if(!m)
        throw Atomsim_exception("Can't initialize memory; mem pointer == null");

    // Initialize Memory object from input ELF File
    ELFIO::elfio reader;

    // Load file into elf reader
    if (!reader.load(filepath)) 
    {
        throwError("INIT0", "Can't find or process ELF file : " + filepath + "\n", true);
    }

    // Check ELF Class, Endiness & segment count
    if(reader.get_class() !=ELFCLASS32)
        throwError("INIT1", "Elf file format invalid: should be 32-bit elf\n", true);
    if(reader.get_encoding() != ELFDATA2LSB)
        throwError("INIT2", "Elf file format invalid: should be little Endian\n", true);

    ELFIO::Elf_Half seg_num = reader.segments.size();

    if(seg_num == 0)
        throwError("INIT3", "Elf file format invalid: should consist of atleast one section\n", true);


    // Read elf and initialize memory
    // if(verbose_flag)
    std::cout << "Segments found : "<< seg_num <<"\n";
    
    unsigned int i = 0;
    while (i < seg_num) // iterate over all segments
    {
        const ELFIO::segment * seg = reader.segments[i];

        // Get segment properties
        
        if (seg->get_type() == SHT_PROGBITS)
        {
            int seg_flags = reader.segments[i]->get_flags();

            if(flag_signatures.end() != std::find(flag_signatures.begin(), flag_signatures.end(), seg_flags))	// Flag found in signature list
            {

                const char* seg_data = reader.segments[i]->get_data();
                const uint seg_size = reader.segments[i]->get_file_size();
                ELFIO::Elf64_Addr seg_strt_addr = reader.segments[i]->get_physical_address();

                if (seg_strt_addr >= m->get_base_addr()  && seg_strt_addr+seg_size <= m->get_base_addr()+m->get_size())
                {
                    // if(verbose_flag)
                    printf("Loading Segment %d @ 0x%08x --- ", i, (unsigned int) reader.segments[i]->get_physical_address());
                    
                    m->store(seg_strt_addr, (uint8_t*)seg_data, seg_size);
                    
                    // while(offset<seg_size)
                    // {
                    //     storeByte(seg_strt_addr + offset, seg_data[offset]);
                    //     offset++;
                    // }

                    // if(verbose_flag)
                    printf("done\n");
                }
            }
        }
        i++;
    }
    return (unsigned int) reader.get_entry();	
}