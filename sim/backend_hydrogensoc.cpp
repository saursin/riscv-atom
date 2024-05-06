#include "backend_hydrogensoc.hpp"

#include "atomsim.hpp"
#include "memory.hpp"
#include "vuart.hpp"
#include "bitbang_uart.hpp"
#include "except.hpp"
#include "rvdefs.hpp"

#include "build/verilated/VHydrogenSoC_headers.h"

#ifdef DBG
#define D(x) x
#else
#define D(x)
#endif

#include "elfio/elfio.hpp"

// ROM
#define ROM_ADDR 0x00010000
#define ROM_SIZE 8192   // 8 KB

// RAM
#define RAM_ADDR 0x20000000
#define RAM_SIZE 49152  // 48 KB

#define BBUART_FRATIO 3
#define BOOTMODE_PIN_OFFSET 8


Backend_atomsim::Backend_atomsim(Atomsim *sim, Backend_config config) : Backend(sim),
                                                                        config_(config),
                                                                        using_vuart_(config.vuart_portname != "")
{
    // generate image file by converting ELF file
    char *varval = getenv("RVATOM");
    if (!varval)
        throw Atomsim_exception("cant find $RVATOM env variable");
    std::string rvatom(varval);

    std::string tmp_bin_file = ".atomsim_ram_img.bin";
    std::string cmd_output = GetStdoutFromCommand("python3 " + rvatom + "/scripts/convelf.py -t elf -m=ram:"+std::to_string(RAM_ADDR)+":"+std::to_string(RAM_SIZE)+":b:"+tmp_bin_file+" "+sim_->sim_config_.ifile, true);
    if (cmd_output.length() > 0) {
        throw Atomsim_exception(cmd_output);
    }
    
    std::vector<char> imgcontents = fReadBin(tmp_bin_file);

    std::string cmd_output1 = GetStdoutFromCommand("rm -f "+tmp_bin_file, true);
    if (cmd_output1.length() > 0) {
        throw Atomsim_exception(cmd_output1);
    }

    // Construct Testbench object
    tb = new Testbench<VHydrogenSoC>();

    // Construct reg map
    regs_.push_back({.name="pc", .alt_name="", .width=R32, .ptr=(void *)&tb->m_core->HydrogenSoC->atom_wb_core->atom_core->ProgramCounter_Old, .is_arch_reg=false});
    regs_.push_back({.name="ir", .alt_name="", .width=R32, .ptr=(void *)&tb->m_core->HydrogenSoC->atom_wb_core->atom_core->InstructionRegister, .is_arch_reg=false});
    for (int i=0; i<32; i++) {
        std::string regname = "x"+std::to_string(i);
        regs_.push_back({.name=regname, .alt_name=rv_abi_regnames[i], .width=R32, .ptr=(void *)&tb->m_core->HydrogenSoC->atom_wb_core->atom_core->rf->regs[i], .is_arch_reg=true});
    }

    // ====== Initialize ========
    // init ram
    if(sim_->sim_config_.verbose_flag) 
        std::cout << "Initializing ram" << std::endl;
    store(RAM_ADDR, (uint8_t*)imgcontents.data(), imgcontents.size());

    // Initialize CPU state by resetting
    reset();

    // ====== Initialize BBUart ========
    bb_uart_ = new BitbangUART((bool *)&tb->m_core->uart_tx_o, (bool *)&tb->m_core->uart_rx_i, BBUART_FRATIO);

    // create a new vuart object
    if (using_vuart_)
    {
        vuart_ = new Vuart(config_.vuart_portname, config_.vuart_baudrate);

        // Clean recieve buffer
        vuart_->clean_recieve_buffer();

        if (sim_->sim_config_.verbose_flag)
            std::cout << "Connected to VUART (" + config_.vuart_portname + ") at " + std::to_string(config_.vuart_baudrate) + " bps" << std::endl;
    }
    else
    {
        if (config_.enable_uart_dump && sim_->sim_config_.verbose_flag)
            std::cout << "Relaying uart-tx to stdout (Note: This mode does not support uart-rx)" << std::endl;
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
        if (vuart_ != nullptr)
            delete vuart_;
    }

    delete bb_uart_;
    delete tb;
}


void Backend_atomsim::UART()
{
    // RX
    if (!bb_uart_->rx_fifo.empty())
    {
        char tchar = bb_uart_->rx_fifo.front();
        if (using_vuart_)
            vuart_->send(tchar); // Redirect to Virtual UART
        
        if (config_.enable_uart_dump)
            std::cout << tchar << std::flush; // Echo on stdout

        bb_uart_->rx_fifo.pop();
    }

    // TX
    if (using_vuart_)
    {
        int rchar = vuart_->recieve();
        if (rchar != (int)-1)
            bb_uart_->tx_fifo.push(rchar);
    }

    // evaluate bbuart state machine for current cycle
    bb_uart_->eval();
}

int Backend_atomsim::tick()
{
    // return if backend finished
    if (this->done()) {
        return 1;
    }

    // Force the bootmode switch value
    tb->m_core->gpio_io = (0b11 & config_.bootmode) << BOOTMODE_PIN_OFFSET;

    // perform uart transaction (if any)
    UART();

    // Tick clock once
    tb->tick();

    return 0;
}

void Backend_atomsim::fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz) {
    if (start_addr >= ROM_ADDR && start_addr < (ROM_ADDR + ROM_SIZE)) 
    {
        if(!(start_addr + buf_sz - 1 < ROM_ADDR + ROM_SIZE))
            throw Atomsim_exception("can't fetch; bufsize too large for mem");

        // Copy mem to buf
        for(unsigned buf_indx = 0; buf_indx < buf_sz; buf_indx++){
            uint32_t mem_addr = start_addr + buf_indx;
            uint32_t mem_indx = (mem_addr-ROM_ADDR) / 4;
            uint32_t line_offset = 8 * ((mem_addr-ROM_ADDR) % 4);
            buf[buf_indx] = 0xff & (tb->m_core->HydrogenSoC->bootrom->mem[mem_indx] >> line_offset);
        }
    }
    else if (start_addr >= RAM_ADDR && start_addr < (RAM_ADDR + RAM_SIZE)) 
    {
        if(!(start_addr + buf_sz - 1 < RAM_ADDR + RAM_SIZE))
            throw Atomsim_exception("can't fetch; bufsize too large for mem");

        // Copy mem to buf
        for(unsigned buf_indx = 0; buf_indx < buf_sz; buf_indx++){
            uint32_t mem_addr = start_addr + buf_indx;
            uint32_t mem_indx = (mem_addr-RAM_ADDR) / 4;
            uint32_t line_offset = 8 * ((mem_addr-RAM_ADDR) % 4);
            buf[buf_indx] = 0xff & (tb->m_core->HydrogenSoC->ram->mem[mem_indx] >> line_offset);
        }
    }
    else {
        char hx[10];
        sprintf(hx, "%08x", start_addr);
        throw Atomsim_exception("memory fetch failed: no mem block at given address (0x"+std::string(hx)+")");
    }
}

void Backend_atomsim::store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)     // TODO: Use these
{
    if (start_addr >= ROM_ADDR && start_addr < (ROM_ADDR + ROM_SIZE)) 
    {
        if(!(start_addr + buf_sz - 1 < ROM_ADDR + ROM_SIZE))
            throw Atomsim_exception("can't store; bufsize too large for mem");
        
        // Copy mem to buf
        for(unsigned buf_indx = 0; buf_indx < buf_sz; buf_indx++){
            uint32_t mem_addr = start_addr + buf_indx;
            uint32_t mem_indx = (mem_addr-ROM_ADDR) / 4;
            uint32_t line_offset = 8 * ((mem_addr-ROM_ADDR) % 4);
            tb->m_core->HydrogenSoC->bootrom->mem[mem_indx] = (tb->m_core->HydrogenSoC->bootrom->mem[mem_indx] & ~(0xff << line_offset)) 
                                                            | (((uint32_t) buf[buf_indx]) << line_offset);
        }
    }
    else if (start_addr >= RAM_ADDR && start_addr < (RAM_ADDR + RAM_SIZE)) 
    {
        if(!(start_addr + buf_sz - 1 < RAM_ADDR + RAM_SIZE))
            throw Atomsim_exception("can't store; bufsize too large for mem");

        // Copy mem to buf
        for(unsigned buf_indx = 0; buf_indx < buf_sz; buf_indx++){
            uint32_t mem_addr = start_addr + buf_indx;
            uint32_t mem_indx = (mem_addr-RAM_ADDR) / 4;
            uint32_t line_offset = 8 * ((mem_addr-RAM_ADDR) % 4);
            tb->m_core->HydrogenSoC->ram->mem[mem_indx] = (tb->m_core->HydrogenSoC->ram->mem[mem_indx] & ~(0xff << line_offset)) 
                                                        | (((uint32_t) buf[buf_indx]) << line_offset);
        }
    }
    else {
        char hx[10];
        sprintf(hx, "%08x", start_addr);
        throw Atomsim_exception("memory store failed: no mem block at given address (0x"+std::string(hx)+")");
    }
}