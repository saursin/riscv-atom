#include "backend_atombones.hpp"

#include "atomsim.hpp"
#include "memory.hpp"
#include "vuart.hpp"
#include "except.hpp"
#include "util.hpp"
#include "rvdefs.hpp"

#include "build/verilated/VAtomBones_headers.h"

#include "elfio/elfio.hpp"

#define UART_ADDR 0x40000000

Backend_atomsim::Backend_atomsim(Atomsim * sim, Backend_config config):
    Backend(sim),
    config_(config),
    using_vuart_(config.vuart_portname != "")
{
    // Construct Testbench object
    tb = new Testbench<VAtomBones>();

    // Constuct Memory objects
    try
    {
        mem_["bootrom"] = std::shared_ptr<Memory> (new Memory(config_.bootrom_size_kb * 1024, config_.bootrom_offset, true));
        mem_["ram"] = std::shared_ptr<Memory> (new Memory(config_.ram_size_kb * 1024, config_.ram_offset, false));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    // Construct reg map
    regs_.push_back({.name="pc", .alt_name="", .width=R32, .ptr=(void *)&tb->m_core->AtomBones->atom_core->ProgramCounter_Old, .is_arch_reg=false});
    regs_.push_back({.name="ir", .alt_name="", .width=R32, .ptr=(void *)&tb->m_core->AtomBones->atom_core->InstructionRegister, .is_arch_reg=false});
    for (int i=0; i<32; i++) {
        std::string regname = "x"+std::to_string(i);
        regs_.push_back({.name=regname, .alt_name=rv_abi_regnames[i], .width=R32, .ptr=(void *)&tb->m_core->AtomBones->atom_core->rf->regs[i], .is_arch_reg=true});
    }
    
    // ====== Initialize ========
    // Initialize memory

    if(sim_->sim_config_.verbose_flag) 
        std::cout << "Initializing bootrom" << std::endl;
    
    mem_["bootrom"]->set_write_protect(false);
    init_from_imgfile(mem_["bootrom"].get(), resolve_envvar_in_path(config_.bootrom_img));
    mem_["bootrom"]->set_write_protect(true);

    if(sim_->sim_config_.verbose_flag) 
        std::cout << "Initializing ram:" << std::endl;
    init_from_elf(mem_["ram"].get(), sim_->sim_config_.ifile, std::vector<int>{5, 6});
       

    // Initialize CPU state by resetting
    reset();

    // ====== Initialize Communication ========

    if(using_vuart_)
    {	
        // create a new vuart object
        vuart_ = new Vuart(config_.vuart_portname, config_.vuart_baudrate);
            
        // Clean recieve buffer
        vuart_->clean_recieve_buffer();

        if(sim_->sim_config_.verbose_flag)
            std::cout << "Connected to VUART ("+config_.vuart_portname+") at "+std::to_string(config_.vuart_baudrate)+" bps" << std::endl;
    }
    else
    {
        if (config_.enable_uart_dump && sim_->sim_config_.verbose_flag)
            std::cout << "Relaying uart-rx to stdout (Note: This mode does not support uart-tx)" << std::endl;
    }

    if (sim_->sim_config_.verbose_flag)
        std::cout << "Initialization complete!\n";
    
    if(config_.enable_uart_dump)
        std::cout << "----------8<-----------8<-----------8<-----------8<---------" << std::endl;
}


Backend_atomsim::~Backend_atomsim()
{
    if (using_vuart_)
    {
        // destroy vuart object
        if(vuart_ != nullptr)
            delete vuart_;
    }

    delete tb;
    mem_.clear();
}


void Backend_atomsim::service_mem_req()
{    
    // Clear all ack signals
    tb->m_core->iport_ack_i = 0;
    tb->m_core->dport_ack_i = 0;

    // ===== Imem Port Reads =====
    uint32_t iaddr = tb->m_core->iport_addr_o & 0xfffffffc;
    if(tb->m_core->iport_valid_o == 1)
    {   
        Word_alias i_w;
        this->fetch(iaddr, i_w.byte, 4);
        tb->m_core->iport_data_i = i_w.word;
        tb->m_core->iport_ack_i = 1;
    }

    // ===== Dmem Port Reads/Writes =====
    uint32_t daddr = tb->m_core->dport_addr_o & 0xfffffffc;
    if(tb->m_core->dport_valid_o)
    {
        if(tb->m_core->dport_we_o)	// *** Writes ***
        {
            Word_alias data_w = {.word = (uint32_t)tb->m_core->dport_data_o};
            if(daddr == UART_ADDR)  // Handle Writes to UART
            {
                if(tb->m_core->dport_sel_o & 0b0001)
                {
                    if(using_vuart_)
                        vuart_->send(data_w.byte[0]);   // Redirect to Virtual UART
                    
                    if(config_.enable_uart_dump)
                        std::cout << data_w.byte[0] << std::flush; // Echo on stdout
                }
            }
            else                    // Handle Writes
            {
                if(tb->m_core->dport_sel_o & 0b0001) 
                    this->store(daddr, &data_w.byte[0], 1);
                if(tb->m_core->dport_sel_o & 0b0010) 
                    this->store(daddr+1, &data_w.byte[1], 1);
                if(tb->m_core->dport_sel_o & 0b0100) 
                    this->store(daddr+2, &data_w.byte[2], 1);
                if(tb->m_core->dport_sel_o & 0b1000) 
                    this->store(daddr+3, &data_w.byte[3], 1);
            }
        }
        else
        {
            Word_alias data_w;
            if(daddr == UART_ADDR)  // Handle Reads from Uart
            {
                if(using_vuart_)
                    data_w.word = 0xff & (uint32_t) vuart_->recieve();
                else
                    data_w.word = (uint32_t) -1;
            }
            else                    // Handle reads
            {
                
                this->fetch(daddr, data_w.byte, 4);
            }
            tb->m_core->dport_data_i = data_w.word;
        }
        tb->m_core->dport_ack_i = 1;
    }
}


int Backend_atomsim::tick()
{
    // return if backend finished
    if(this->done()) {
        return 1;
    }

    // Service Memory Request
    service_mem_req();
    
    // Tick clock once
    tb->tick();

    return 0;
}


void Backend_atomsim::fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)
{
    bool success = false;
    for (auto mem_block: mem_)  // search for mem blk
    {
        std::shared_ptr<Memory> m = mem_block.second;
        if(m->addr_in_range(start_addr))
        {
            if(!m->block_in_range(start_addr, buf_sz))
                throw Atomsim_exception("can't fetch; bufsize too large for mem");
            
            m->fetch(start_addr, buf, buf_sz);
            success = true;
            break;  // exit search loop
        }
    }

    if (!success)
    {
        char hx[10];
        sprintf(hx, "%08x", start_addr);
        throw Atomsim_exception("memory fetch failed: no mem block at given address (0x"+std::string(hx)+")");
    }
}
   

void Backend_atomsim::store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)
{
    bool success = false;
    for (auto mem_block: mem_)  // search for mem blk
    {
        std::shared_ptr<Memory> m = mem_block.second;
        if(m->addr_in_range(start_addr))
        {
            if(!m->block_in_range(start_addr, buf_sz))
                throw Atomsim_exception("can't store; bufsize too large for mem");
            
            m->store(start_addr, buf, buf_sz);
            success = true;
            break;  // exit search loop
        }
    }

    if (!success)
    {
        char hx[10];
        sprintf(hx, "%08x", start_addr);
        throw Atomsim_exception("memory store failed: no mem block at given address (0x"+std::string(hx)+")");
    }
}
