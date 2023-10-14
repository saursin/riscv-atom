#include "backend_atombones.hpp"

#include "atomsim.hpp"
#include "simstate.hpp"
// #include "testbench.hpp"
#include "memory.hpp"
#include "vuart.hpp"
#include "except.hpp"
#include "util.hpp"

#include "build/verilated/VAtomBones_headers.h"

#include "elfio/elfio.hpp"

#define UART_ADDR 0x40000000

#define RV_INSTR_EBREAK 0x100073


Backend_atomsim::Backend_atomsim(Atomsim * sim, Backend_config config):
    Backend(sim, &(sim->simstate_)),
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


void Backend_atomsim::refresh_state()
{
    // Get PC
    simstate_->state_.pc_f = tb->m_core->AtomBones->atom_core->ProgramCounter;
    simstate_->state_.pc_e = tb->m_core->AtomBones->atom_core->ProgramCounter_Old;

    // Get IR
    simstate_->state_.ins_e = tb->m_core->AtomBones->atom_core->InstructionRegister;

    // Get Regs
    for(int i=0; i<32; i++)
    {
        simstate_->state_.rf[i] = tb->m_core->AtomBones->atom_core->rf->regs[i];
    }

    // Get Signals
    simstate_->signals_.jump_decision = tb->m_core->AtomBones->atom_core->__PVT__jump_decision;

    // get Tickcounts
    simstate_->state_.tickcount = tb->m_tickcount;
    simstate_->state_.tickcount_total = tb->m_tickcount_total;
}


// void Backend_atomsim::UART()
// {
    
//     // --------- Atom->port -----------
//     /*
//         Since in classical single wishbone write transaction, wb_we pin remains asserted until 
//         the	tansaction is marked finish by the slave by setting the wb_ack pin. From prespective
//         of UART, it sees the we pin high for multiple cycles and it may mistakenly infer it
//         as multiple rreads/writes to same addrress. This piece of logic is to prevent that.
//         When we find 'we' asserted, we read the value and wait for 2 cycles after it. This 
//         prevents multiple reads of data in same transaction.
//     */		
//     static int wait = 0;
//     if(wait==0 && tb->m_core->dmem_valid_o && tb->m_core->dmem_we_o && tb->m_core->dmem_addr_o==UART_ADDR && tb->m_core->dmem_sel_o & 0b0001)
//     {
//         char c = (char)tb->m_core->dmem_data_o;

//         if (using_vuart_)
//             vuart_->send(c);
//         else
//             std::cout << c;

//         wait=2;	// Wait for 2 cycles
//     } 
    
//     if(wait>0)
//     {
//         wait--;
//     }

//     // --------- Port->atom -----------
//     /*
//         This section of code deals with port to sim uart communication. port->recieve() is 
//         called in every sim cycle (sim tick). value of recieve buffer is stored in 'recv' 
//         variable. Now, if (recv!=-1) i.e. a valid character is present, it is stored in the 
//         dummy hardware register of simpluart_wb, and set bit[0] of status register.
//     */
//     static int recv;
//     if(using_vuart_)
//     {
//         recv = vuart_->recieve();
//     }
//     else
//     {
//         recv = (int)-1;
//     }	
            
//     if(recv != (int)-1)	// something recieved
//     {
//         uint8_t b[] = {(uint8_t)recv, 0b1};
//         mem_["pmem"]->store(UART_ADDR, b, 2);
//     }


//     // Action to be done if core read/wrote to UART registers in this cycle
//     if(tb->m_core->dmem_valid_o && tb->m_core->dmem_addr_o==UART_ADDR && tb->m_core->dmem_sel_o==0b0001)
//     {
//         if(tb->m_core->dmem_we_o)   // tried to write to DREG
//         {
//             // since dreg is a read only register therefore core writes 
//             // should ideally have no effect, so we restore the value to prev value
//             uint8_t b [] = {(uint8_t) recv};
//             mem_["pmem"]->store(UART_ADDR, b, 1);
//         }
//         else                        // tried to read from DREG
//         {
//             // clear status reg bit[0]
//             uint8_t b[] = {0b0};
//             mem_["pmem"]->store(UART_ADDR + 1, b, 1);
//         }
//     }
// }


int Backend_atomsim::tick()
{
    if(this->done())
    {
        return 1;
    }

    // Service Memory Request
    service_mem_req();
    
    // perform uart transaction (if any)
    // UART();

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
    if (tb->m_core->AtomBones->atom_core->InstructionRegister == RV_INSTR_EBREAK)  // can't rely on stimstate.state.ins_e cuz that's only updated conditionally
    {
        // ============ REGISTER FILE DUMP (For SCAR) ==============
        if(sim_->sim_config_.dump_on_ebreak_flag)
        {
            simstate_->dump_simstate(sim_->sim_config_.dump_file);
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
                Word_alias w;
                this->fetch(addr, w.byte, 4);

                char temp [50];
                sprintf(temp, "%08x", w.word);
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