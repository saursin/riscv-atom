#include "backend_hydrogensoc.hpp"

#include "atomsim.hpp"
#include "memory.hpp"
#include "vuart.hpp"
#include "bitbang_uart.hpp"
#include "spi_flash.hpp"
#include "sd_card.hpp"
#include "except.hpp"
#include "rvdefs.hpp"

#include "VHydrogenSoC_sim_headers.h"

#ifdef DBG
#define D(x) x
#else
#define D(x)
#endif

#include "elfio/elfio.hpp"

// Memory map from the soctarget config (SOC_<X>_ADDR/SIZE defines)
#define ROM_ADDR SOC_BOOTROM_ADDR
#define ROM_SIZE SOC_BOOTROM_SIZE
#define RAM_ADDR SOC_RAM_ADDR
#define RAM_SIZE SOC_RAM_SIZE

#define BOOTMODE_PIN_OFFSET 8


Backend_atomsim::Backend_atomsim(Atomsim *sim, Backend_config config) : Backend(sim),
                                                                        config_(config)
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
    tb = new Testbench<VHydrogenSoC_sim>();

    // Construct reg map
    regs_.push_back({.name="pc", .alt_name="", .width=R32, .ptr=(void *)&tb->m_core->HydrogenSoC_sim->soc->atom_wb_core->atom_core->ProgramCounter_Old, .is_arch_reg=false});
    regs_.push_back({.name="ir", .alt_name="", .width=R32, .ptr=(void *)&tb->m_core->HydrogenSoC_sim->soc->atom_wb_core->atom_core->InstructionRegister, .is_arch_reg=false});
    for (int i=0; i<32; i++) {
        std::string regname = "x"+std::to_string(i);
        regs_.push_back({.name=regname, .alt_name=rv_abi_regnames[i], .width=R32, .ptr=(void *)&tb->m_core->HydrogenSoC_sim->soc->atom_wb_core->atom_core->rf->regs[i], .is_arch_reg=true});
    }

    // ====== Initialize ========
    // init ram
    if(sim_->sim_config_.verbose_flag) 
        std::cout << "Initializing ram" << std::endl;
    store(RAM_ADDR, (uint8_t*)imgcontents.data(), imgcontents.size());

    // Initialize CPU state by resetting
    reset();

    // ====== Devices on the SoC's pins ========
    // a terminal on the UART: the virtual UART (pty or serial port) and/or stdout
    std::unique_ptr<Vuart> vuart;
    if (!config_.vuart_portname.empty())
    {
        vuart = std::make_unique<Vuart>(config_.vuart_portname, config_.vuart_baudrate, !config_.vuart_no_wait);
        vuart->clean_recieve_buffer();

        if (sim_->sim_config_.verbose_flag)
            std::cout << "Connected to VUART (" + config_.vuart_portname + ") at " + std::to_string(config_.vuart_baudrate) + " bps" << std::endl;
    }
    else if (config_.enable_uart_dump && sim_->sim_config_.verbose_flag)
    {
        std::cout << "Relaying uart-tx to stdout (Note: This mode does not support uart-rx)" << std::endl;
    }
    devices_.push_back(std::make_unique<BitbangUART>(&tb->m_core->uart_tx_o, &tb->m_core->uart_rx_i,
                                                     &tb->m_core->HydrogenSoC_sim->soc->uart->reg_div,
                                                     std::move(vuart), config_.enable_uart_dump));

    // SPI bus: the boot flash on chip select 0, the SD card on 1
#ifdef SOC_EN_SPI
    if (!config_.spi_flash_image.empty())
    {
        devices_.push_back(std::make_unique<SPIFlash>(&tb->m_core->spi_sck_o, &tb->m_core->spi_mosi_o,
                                                      &tb->m_core->spi_cs_o, 0, &tb->m_core->spi_miso_i,
                                                      config_.spi_flash_image));
        if (sim_->sim_config_.verbose_flag)
            std::cout << "SPI flash on chip select 0: " << config_.spi_flash_image << std::endl;
    }
    if (!config_.sdcard_image.empty())
    {
        if (SOC_SPI_NUM_CS < 2)
            throw Atomsim_exception("--sdcard: the SD card goes on SPI chip select 1; this config has one");
        devices_.push_back(std::make_unique<SDCard>(&tb->m_core->spi_sck_o, &tb->m_core->spi_mosi_o,
                                                    &tb->m_core->spi_cs_o, 1, &tb->m_core->spi_miso_i,
                                                    config_.sdcard_image));
        if (sim_->sim_config_.verbose_flag)
            std::cout << "SD card on chip select 1: " << config_.sdcard_image << std::endl;
    }
#else
    if (!config_.spi_flash_image.empty() || !config_.sdcard_image.empty())
        throw Atomsim_exception("--spi-flash/--sdcard: this config has no SPI");
#endif

    if (sim_->sim_config_.verbose_flag)
        std::cout << "Initialization complete!\n";
    
    if(config_.enable_uart_dump)
        std::cout << "----------8<-----------8<-----------8<-----------8<---------" << std::endl;
}

Backend_atomsim::~Backend_atomsim()
{
    devices_.clear();   // before the testbench they are wired to
    delete tb;
}


int Backend_atomsim::tick()
{
    // return if backend finished
    if (this->done()) {
        return 1;
    }

    // Force the bootmode switch value
    tb->m_core->gpio_inp_i = (0b11 & config_.bootmode) << BOOTMODE_PIN_OFFSET;

#ifdef SOC_EN_SPI
    tb->m_core->spi_miso_i = 1;     // pull-up: a selected SPI device overrides it
#endif
    for (auto &d : devices_)
        d->eval();

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
            buf[buf_indx] = 0xff & (tb->m_core->HydrogenSoC_sim->soc->bootrom->mem[mem_indx] >> line_offset);
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
            buf[buf_indx] = 0xff & (tb->m_core->HydrogenSoC_sim->soc->ram->mem[mem_indx] >> line_offset);
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
            tb->m_core->HydrogenSoC_sim->soc->bootrom->mem[mem_indx] = (tb->m_core->HydrogenSoC_sim->soc->bootrom->mem[mem_indx] & ~(0xff << line_offset)) 
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
            tb->m_core->HydrogenSoC_sim->soc->ram->mem[mem_indx] = (tb->m_core->HydrogenSoC_sim->soc->ram->mem[mem_indx] & ~(0xff << line_offset)) 
                                                        | (((uint32_t) buf[buf_indx]) << line_offset);
        }
    }
    else {
        char hx[10];
        sprintf(hx, "%08x", start_addr);
        throw Atomsim_exception("memory store failed: no mem block at given address (0x"+std::string(hx)+")");
    }
}