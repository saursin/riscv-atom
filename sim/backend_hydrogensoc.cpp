#include "backend_hydrogensoc.hpp"

#include "atomsim.hpp"
#include "simstate.hpp"
// #include "testbench.hpp"
#include "memory.hpp"
#include "vuart.hpp"
#include "bitbang_uart.hpp"
#include "except.hpp"
// #include "util.hpp"

#include "build/verilated/VHydrogenSoC_headers.h"

#ifdef DBG
#define D(x) x
#else
#define D(x)
#endif

#include "elfio/elfio.hpp"

#define RV_INSTR_EBREAK 0x100073

// ROM
#define ROM_ADDR 0x00010000
#define ROM_SIZE 8192   // 8 KB

// RAM
#define RAM_ADDR 0x20000000
#define RAM_SIZE 49152  // 48 KB

#define BBUART_FRATIO 3

Backend_atomsim::Backend_atomsim(Atomsim *sim, Backend_config config) : Backend(sim, &(sim->simstate_)),
                                                                        config_(config),
                                                                        using_vuart_(config.vuart_portname != "")
{
    // generate image files by converting ELF file to memory image files
    char *varval = getenv("RVATOM");
    if (!varval)
        throw Atomsim_exception("cant find $RVATOM env variable");
    std::string rvatom(varval);

    std::string cmd_output = GetStdoutFromCommand("python3 " + rvatom + "/scripts/convelf.py -t elf -j " + rvatom + "/hydrogensoc.json --keep-temp " + sim_->sim_config_.ifile, true);
    if (cmd_output.length() > 0)
    {
        throw Atomsim_exception(cmd_output);
    }

    // Construct Testbench object
    tb = new Testbench<VHydrogenSoC>();

    // ====== Initialize ========
    // Initialize CPU state by resetting
    this->reset();
    tb->m_core->eval();

    // get initial signal values
    this->refresh_state();

    // ====== Initialize Communication ========
    bb_uart_ = new BitbangUART((bool *)&tb->m_core->uart_usb_tx_o, (bool *)&tb->m_core->uart_usb_rx_i, BBUART_FRATIO);

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
            std::cout << "Relaying uart-rx to stdout (Note: This mode does not support uart-rx)" << std::endl;
    }

    if (sim_->sim_config_.verbose_flag)
        std::cout << "Initialization complete!\n";
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

void Backend_atomsim::refresh_state()
{
    // Get PC
    simstate_->state_.pc_f = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->ProgramCounter;
    simstate_->state_.pc_e = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->ProgramCounter_Old;

    // Get IR
    simstate_->state_.ins_e = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->InstructionRegister;

    // Get Regs
    for (int i = 0; i < 32; i++)
        simstate_->state_.rf[i] = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->rf->regs[i];

    // Get Signals
    simstate_->signals_.jump_decision = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->__PVT__jump_decision;

    // get Tickcounts
    simstate_->state_.tickcount = tb->m_tickcount;
    simstate_->state_.tickcount_total = tb->m_tickcount_total;
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
    if (this->done())
    {
        return 1;
    }

    // perform uart transaction (if any)
    UART();

    // Tick clock once
    tb->tick();

    // Refresh Data
    if (sim_->sim_config_.debug_flag || sim_->sim_config_.dump_on_ebreak_flag)
    {
        /* We need to refresh state data to print debug screen in debug mode and
         * to keep track of instruction register being equal to ebreak, \
         * in case dump_on_ebreak is set
         */
        refresh_state();
    }

    if (sim_->sim_config_.debug_flag)
        sim_->display_dbg_screen();

    // ===== Check Hault Condition =====
    if (tb->m_core->HydrogenSoC->atom_wb_core->atom_core->InstructionRegister == RV_INSTR_EBREAK) // can't rely on stimstate.state.ins_e cuz that's only updated conditionally
    {
        // ============ REGISTER FILE DUMP (For SCAR) ==============
        if(sim_->sim_config_.dump_on_ebreak_flag)
        {
            simstate_->dump_simstate(sim_->sim_config_.dump_file);
        }

        // ==========  MEM SIGNATURE DUMP (For RISC-V-Arch Tests) =============
        if (sim_->sim_config_.signature_file.length() != 0)
        {
            // Get start and end address of signature
            long int begin_signature_at = -1;
            long int end_signature_at = -1;

            ELFIO::elfio reader;

            if (!reader.load(sim_->sim_config_.ifile))
            {
                throwError("SIG", "Can't find or process ELF file : " + sim_->sim_config_.ifile + "\n", true);
            }

            ELFIO::Elf_Half n = reader.sections.size();
            for (ELFIO::Elf_Half i = 0; i < n; ++i) // For all sections
            {
                ELFIO::section *sec = reader.sections[i];
                if (SHT_SYMTAB == sec->get_type() || SHT_DYNSYM == sec->get_type())
                {
                    ELFIO::symbol_section_accessor symbols(reader, sec);
                    ELFIO::Elf_Xword sym_no = symbols.get_symbols_num();
                    if (sym_no > 0)
                    {
                        for (ELFIO::Elf_Xword i = 0; i < sym_no; ++i)
                        {
                            std::string name;
                            ELFIO::Elf64_Addr value = 0;
                            ELFIO::Elf_Xword size = 0;
                            unsigned char bind = 0;
                            unsigned char type = 0;
                            ELFIO::Elf_Half section = 0;
                            unsigned char other = 0;
                            symbols.get_symbol(i, name, value, size, bind, type,
                                               section, other);

                            if (name == "begin_signature")
                                begin_signature_at = value;
                            if (name == "end_signature")
                                end_signature_at = value;
                        }
                    }
                }
            }

            if (begin_signature_at == -1 || end_signature_at == -1)
            {
                throwError("SIG", "One or both of 'begin_signature' & 'end_signature' symbols missing from ELF symbol table: " + sim_->sim_config_.ifile + "\n", true);
            }

            //  dump data to signature file
            std::vector<std::string> fcontents;
            printf("Dumping signature region [0x%08lx-0x%08lx]\n", begin_signature_at, end_signature_at);

            for (uint32_t addr = begin_signature_at; addr < end_signature_at; addr += 4)
            {
                uint32_t index = addr - RAM_ADDR;

                if (!(index > 0 && index < RAM_SIZE - 4)) // check bounds
                    throw Atomsim_exception("Signature out of bounds" + std::to_string(addr));

                uint32_t value = (uint32_t)tb->m_core->HydrogenSoC->ram->mem[addr / 4];

                char temp[50];
                sprintf(temp, "%08x", value);
                fcontents.push_back(temp);
            }
            fWrite(fcontents, sim_->sim_config_.signature_file);
        }

        if (sim_->sim_config_.verbose_flag)
            std::cout << "Haulting @ tick " << tb->m_tickcount_total << std::endl;
        return 1;
    }
    if (tb->m_tickcount_total > sim_->sim_config_.maxitr)
    {
        throwWarning("SIM0", "Simulation iterations exceeded maxitr(" + std::to_string(sim_->sim_config_.maxitr) + ")\n");
        return 1;
    }

    return 0;
}

// void Backend_atomsim::fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)     // TODO: Use these
// {
//     if((start_addr >= IMEM_ADDR) && (start_addr < IMEM_ADDR+IMEM_SIZE))  // fetch from imem
//     {
//         if(!(start_addr+buf_sz-1 < IMEM_ADDR+IMEM_SIZE))
//             throw Atomsim_exception("can't fetch; bufsize too large for mem");

//         // fetch

//     }
//     else if((start_addr >= DMEM_ADDR) && (start_addr < DMEM_ADDR+DMEM_SIZE))  // fetch from dmem
//     {
//         if(!(start_addr+buf_sz-1 < DMEM_ADDR+DMEM_SIZE))
//             throw Atomsim_exception("can't fetch; bufsize too large for mem");

//         // fetch
//         for(uint32_t addr = start_addr; addr < start_addr+buf_sz; addr+=4)
//         {
//             uint32_t index = addr - DMEM_ADDR;
//             uint32_t value = (uint32_t)tb->m_core->HydrogenSoC->dmem->mem[index/4];
//             // now store this intop the buf
//         }
//     }
//     else
//     {
//         throw Atomsim_exception("memory fetch failed: no mem block at given address");
//     }
// }

// void Backend_atomsim::store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)     // TODO: Use these
// {
//     bool success = false;
//     for (auto mem_block: mem_)  // search for mem blk
//     {
//         std::shared_ptr<Memory> m = mem_block.second;
//         if(m->addr_in_range(start_addr))
//         {
//             if(!m->block_in_range(start_addr, buf_sz))
//                 throw Atomsim_exception("cant store; bufsize too large for mem");

//             m->store(start_addr, buf, buf_sz);
//             success = true;
//             break;  // exit search loop
//         }
//     }

//     if (!success)
//     {
//         throw Atomsim_exception("memory store failed: no mem block at given address");
//     }
// }