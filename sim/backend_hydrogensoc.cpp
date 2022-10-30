#include "backend_hydrogensoc.hpp"

#include "atomsim.hpp"
#include "simstate.hpp"
// #include "testbench.hpp"
#include "memory.hpp"
#include "vuart.hpp"
#include "except.hpp"
// #include "util.hpp"

#include "build/verilated/VHydrogenSoC.h"
#include "build/verilated/VHydrogenSoC_HydrogenSoC.h"
#include "build/verilated/VHydrogenSoC_AtomRV_wb.h"
#include "build/verilated/VHydrogenSoC_AtomRV.h"
#include "build/verilated/VHydrogenSoC_RegisterFile.h"

//#include "build/verilated/VHydrogenSoC_DualPortRAM_wb__Af_Mz1.h"
// #include "build/verilated/VHydrogenSoC_DualPortRAM_wb__pi1.h"
#include "build/verilated/VHydrogenSoC_SinglePortRAM_wb__pi1.h"
#include "build/verilated/VHydrogenSoC_SinglePortRAM_wb__pi2.h"
#include "build/verilated/VHydrogenSoC_UART.h"


#ifdef DBG
#define D(x) x
#else
#define D(x)
#endif

#include "elfio/elfio.hpp"

#define RV_INSTR_EBREAK 0x100073

// ROM
#define ROM_ADDR        0x00010000
#define ROM_SIZE        0x00004000   // 16 KB

// RAM
#define RAM_ADDR        0x20000000
#define RAM_SIZE        0x00008000   // 32 KB

Backend_atomsim::Backend_atomsim(Atomsim * sim, Backend_config config):
    Backend(sim, &(sim->simstate_)),
    config_(config),
    using_vuart_(config.vuart_portname != "")
{
    // generate image files by converting ELF file to memory image files
    char * varval = getenv("RVATOM");
    if(!varval)
        throw Atomsim_exception("cant find $RVATOM env variable");
    std::string rvatom(varval);
    
    std::string cmd_output = GetStdoutFromCommand("python3 "+rvatom+"/scripts/convelf.py -t elf -j "+rvatom+"/hydrogensoc.json --keep-temp " + sim_->sim_config_.ifile, true);
    if(cmd_output.length() > 0)
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

    // create a new vuart object
    if(using_vuart_)
    {	
        vuart_ = new Vuart(config_.vuart_portname, config_.vuart_baudrate);
            
        // Clean recieve buffer
        vuart_->clean_recieve_buffer();

        if(sim_->sim_config_.verbose_flag)
            std::cout << "Connected to VUART ("+config_.vuart_portname+") at "+std::to_string(config_.vuart_baudrate)+" bps" << std::endl;
    }
    else
    {
        if(sim_->sim_config_.verbose_flag)
            std::cout << "Relaying uart-rx to stdout (Note: This mode does not support uart-tx)" << std::endl;
    }

    if (sim_->sim_config_.verbose_flag)
        std::cout << "Initialization complete!\n";
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
}


void Backend_atomsim::refresh_state()
{
    // Get PC
    simstate_->state_.pc_f = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->ProgramCounter;
    simstate_->state_.pc_e = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->ProgramCounter_Old;

    // Get IR
    simstate_->state_.ins_e = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->InstructionRegister;

    // Get Regs
    for(int i=0; i<32; i++)
    {
        if(i==0)
            simstate_->state_.rf[i] = 0;
        else
            simstate_->state_.rf[i] = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->rf->regs[i-1];
    }

    // Get Signals
    simstate_->signals_.jump_decision = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->__PVT__jump_decision;

    // get Tickcounts
    simstate_->state_.tickcount = tb->m_tickcount;
    simstate_->state_.tickcount_total = tb->m_tickcount_total;
}


void Backend_atomsim::UART()
{
    typedef enum {
        IDLE,
        START_BIT,
        DATA_BITS,
        STOP_BIT
    } UART_State;

    // const int baud = 115200;         // For real-time simulation
    // int FR = 12000000/(baud + 2);

    int FR = 3; // Fastest Possible for Simulation

    //////////////////////// RECIEVE ///////////////////////////
    static bool recv_prev_tx = false;       // prev tx pin value
    static UART_State recv_state = IDLE;    // current state
    static int recv_wait_cyc = 0;           // number of cycles to wait
    static int recv_got_bits = 0;           // number of bits received
    static uint8_t recv_byte = 0;           // data byte

    // Sample TX
    bool tx = tb->m_core->uart_usb_tx_o;

    if(recv_wait_cyc){
        D(printf("wait %d\n", recv_wait_cyc);)
        recv_wait_cyc--;
    } else {
        switch(recv_state) {
        case IDLE:
            // wait for falling edge on tx
            if (recv_prev_tx==true && tx == false) {
                recv_state = START_BIT;
                recv_byte = 0;
                recv_got_bits = 0;
                D(printf("@%ld: detected negedge on tx\n", tb->get_total_tickcount());)
                recv_wait_cyc = FR/2-1;
            }
            break;
        
        case START_BIT:
            // check start bit
            if (tx == false) {
                D(printf("start_bit: ok\n");)
                recv_state = DATA_BITS;
                recv_wait_cyc = FR-1;
            }
            else {
                D(printf("err: no start bit\n");)
                recv_state = IDLE;
            }
            break;
        
        case DATA_BITS:
            recv_byte = (recv_byte >> 1) | (((uint8_t)tx) << 7);
            D(printf("tx=%d; got_bits=%d; byte: %02x\n", tx, recv_got_bits, recv_byte);)
            recv_got_bits++;

            if(recv_got_bits == 8) {
                recv_state = STOP_BIT;
            }
            else {
                recv_state = DATA_BITS;  // loop
            }
            recv_wait_cyc = FR-1;
            break;
        
        case STOP_BIT:
            // check stop bit
            if(tx==true) {
                D(printf("Recieved Byte! = 0x%02x\n", recv_byte);)
                if (using_vuart_)
                    vuart_->send(recv_byte);
                else
                    std::cout << recv_byte;
                
                recv_wait_cyc = FR/2-1;
                recv_state = IDLE;
            }
            else {
                printf("UART: Framing error, '%c' (0x%02x)\n", recv_byte, recv_byte);
                recv_state = IDLE;
            }
            break;

        default:
            recv_state = IDLE;

        }

    recv_prev_tx = tx;
    }


    // Set RX
    tb->m_core->uart_usb_rx_i = 1;
}


int Backend_atomsim::tick()
{
    if(this->done())
    {
        return 1;
    }
   
    // perform uart transaction (if any)
    UART();

    // Tick clock once
    tb->tick();

    // Refresh Data
    if(sim_->sim_config_.debug_flag || sim_->sim_config_.dump_on_ebreak_flag)
    {
        /* We need to refresh state data to print debug screen in debug mode and
         * to keep track of instruction register being equal to ebreak, \
         * in case dump_on_ebreak is set
         */
        refresh_state();
    }

    if(sim_->sim_config_.debug_flag)
        sim_->display_dbg_screen();


    // ===== Check Hault Condition =====
    if (tb->m_core->HydrogenSoC->atom_wb_core->atom_core->InstructionRegister == RV_INSTR_EBREAK)  // can't rely on stimstate.state.ins_e cuz that's only updated conditionally
    {
        // ============ REGISTER FILE DUMP (For SCAR) ==============
        if(sim_->sim_config_.dump_on_ebreak_flag)
        {
            std::vector<std::string> fcontents;
            
            for(int i=0; i<34; i++)
            {
                char temp [50];
                unsigned int tmpval;

                switch(i-2)
                {
                    case -2: tmpval = simstate_->state_.pc_e; sprintf(temp, "pc 0x%08x", tmpval); break;
                    case -1: tmpval = simstate_->state_.ins_e; sprintf(temp, "ir 0x%08x", tmpval); break;
                    default: tmpval = simstate_->state_.rf[i-2]; sprintf(temp, "x%d 0x%08x",i-2, tmpval); break;
                }
                fcontents.push_back(std::string(temp));
            }


            fWrite(fcontents, sim_->sim_config_.dump_file);
        }
        
        // ==========  MEM SIGNATURE DUMP (For RISC-V-Arch Tests) =============
        if(sim_->sim_config_.signature_file.length()!=0)
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
            for ( ELFIO::Elf_Half i = 0; i < n; ++i )  // For all sections
            {
                ELFIO::section* sec = reader.sections[i];
                if ( SHT_SYMTAB == sec->get_type() || SHT_DYNSYM == sec->get_type() ) 
                {
                    ELFIO::symbol_section_accessor symbols( reader, sec );
                    ELFIO::Elf_Xword sym_no = symbols.get_symbols_num();
                    if ( sym_no > 0 ) 
                    {	
                        for ( ELFIO::Elf_Xword i = 0; i < sym_no; ++i ) {
                            std::string   name;
                            ELFIO::Elf64_Addr value = 0;
                            ELFIO::Elf_Xword size	= 0;
                            unsigned char bind    	= 0;
                            unsigned char type    	= 0;
                            ELFIO::Elf_Half section = 0;
                            unsigned char other   	= 0;
                            symbols.get_symbol( i, name, value, size, bind, type,
                                                section, other );

                            if (name == "begin_signature")
                                begin_signature_at = value;
                            if (name == "end_signature")
                                end_signature_at = value;
                        }
                    }
                }
            }

            if(begin_signature_at == -1 || end_signature_at == -1)
            {
                throwError("SIG", "One or both of 'begin_signature' & 'end_signature' symbols missing from ELF symbol table: " + sim_->sim_config_.ifile + "\n", true);
            }

            //  dump data to signature file
            std::vector<std::string> fcontents;
            printf("Dumping signature region [0x%08lx-0x%08lx]\n", begin_signature_at, end_signature_at);

            for(uint32_t addr=begin_signature_at; addr<end_signature_at; addr+=4)
            {
                uint32_t index = addr - RAM_ADDR;
                
                if (!(index > 0 && index < RAM_SIZE - 4))    // check bounds
                    throw Atomsim_exception("Signature out of bounds"+std::to_string(addr));

                uint32_t value = (uint32_t)tb->m_core->HydrogenSoC->ram->mem[addr/4];

                char temp [50];
                sprintf(temp, "%08x", value);
                fcontents.push_back(temp);
            }
            fWrite(fcontents, sim_->sim_config_.signature_file);
        }

        if (sim_->sim_config_.verbose_flag)
            std::cout << "Haulting @ tick " << tb->m_tickcount_total << std::endl;
        return 1;
    }
    if(tb->m_tickcount_total > sim_->sim_config_.maxitr)
    {
        throwWarning("SIM0", "Simulation iterations exceeded maxitr("+std::to_string(sim_->sim_config_.maxitr)+")\n");
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