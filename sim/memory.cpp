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
    uint32_t rel_start_addr = start_addr - addr_offset_;    // relative to current memory block

    for (uint32_t i = 0; i < buf_sz; i++)
    {
        // get index
        uint32_t indx = rel_start_addr + i;
        
        if (!(indx < size_))    // check bounds
        {
            char except_buf[70];
            sprintf(except_buf, "Can't fetch, address outside range of memory [0x%08x]", start_addr + i);
            throw Atomsim_exception(except_buf);
        }

        // copy from mem
        buf[i] = mem_[indx];
    }
}


void Memory::store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)
{
    if(is_write_protected_)
    {
        char except_buf[70];
        sprintf(except_buf, "Attempted to store in a write-protected memory @ address [0x%08x]", start_addr);
        throw Atomsim_exception(except_buf);
    }

    uint32_t rel_start_addr = start_addr - addr_offset_;    // relative to current memory block

    for (uint32_t i = 0; i < buf_sz; i++)
    {
        // get index
        uint32_t indx = rel_start_addr + i;
        
        if (!(indx < size_))    // check bounds
        {
            char except_buf[70];
            sprintf(except_buf, "Can't store, address outside range of memory [0x%08x]", start_addr + i);
            throw Atomsim_exception(except_buf);
        }

        // copy to mem
        mem_[indx] = buf[i];
    }
}


unsigned init_from_elf(Memory * m, std::string filepath, std::vector<int> flag_signatures)    // TODO: Remove this and switch to init from binary/hex files
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
    //     std::cout << "Segments found : "<< seg_num <<"\n";
    
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
                // Get section
                const ELFIO::Elf64_Addr seg_base_addr = reader.segments[i]->get_physical_address();
                const ELFIO::Elf_Xword seg_size = reader.segments[i]->get_file_size();
                const char* seg_data = reader.segments[i]->get_data();

                // check bounds
                if (seg_base_addr >= m->get_base_addr() && seg_base_addr < m->get_base_addr() + m->get_size() && seg_base_addr+seg_size <= m->get_base_addr()+m->get_size())
                {
                    // initialize memory
                    // printf("mem-base=%08x, sz=%08x\n", m->get_base_addr(), m->get_size());
                    // if(verbose_flag)
                    printf("Loading segment %d [base=0x%08x, sz=% 6d bytes, at=0x%08x] ...\t", i, (unsigned) seg_base_addr, (unsigned) seg_size, (unsigned) seg_base_addr);
                    m->store(seg_base_addr, (uint8_t*)seg_data, seg_size);
                    // if(verbose_flag)
                    printf("done\n");
                }
            }
        }
        i++;
    }
    return (unsigned int) reader.get_entry();
}


void init_from_bin(Memory * m, std::string filepath) {
    if(!m) {
        throw Atomsim_exception("Can't initialize memory; mem pointer == null");
    }

    std::vector<char> fcontents = fReadBin(filepath);

    printf("Loading %ld bytes at 0x%08x from %s\n", fcontents.size(), m->get_base_addr(), filepath.c_str());
    m->store(m->get_base_addr(), (uint8_t*)fcontents.data(), fcontents.size());
}


void init_from_hex(Memory * m, std::string filepath) {
    if(!m) {
        throw Atomsim_exception("Can't initialize memory; mem pointer == null");
    }

    std::vector<std::string> fcontents = fRead(filepath);
    uint32_t addr = m->get_base_addr(); 

    printf("Loading %ld bytes at 0x%08x from %s\n", fcontents.size(), addr, filepath.c_str());

    // parse
    for(unsigned l=0; l<fcontents.size(); l++) {
        std::string line = strip(fcontents[l]);
        if (line.length() != 8)
            throw Atomsim_exception("file:"+filepath+":"+std::to_string(l+1)+" Invalid hex format");
        
        Word_alias wa;
        wa.word = std::stoul(line, nullptr, 16);
        m->store(addr, wa.byte, 4);
        addr += 4;
    }
}


void init_from_imgfile(Memory * m, std::string filepath) {
    std::string ext = filepath.substr(filepath.length()-4, 4);
    if(ext == ".hex")
        init_from_hex(m, filepath);
    else if (ext == ".bin")
        init_from_bin(m, filepath);
    else
        throw Atomsim_exception("invalid image-file format: "+filepath);
}