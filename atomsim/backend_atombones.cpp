#include "backend_atombones.hpp"

#include "atomsim.hpp"
#include "simstate.hpp"
// #include "testbench.hpp"
#include "memory.hpp"
#include "vuart.hpp"
#include "except.hpp"
// #include "util.hpp"

#include "build/verilated/VAtomBones.h"
#include "build/verilated/VAtomBones_AtomBones.h"
#include "build/verilated/VAtomBones_AtomRV.h"
#include "build/verilated/VAtomBones_RegisterFile__R20_RB5.h"

#include "elfio/elfio.hpp"

#define RV_INSTR_EBREAK 0x100073


Backend_atomsim::Backend_atomsim(Atomsim * sim, Backend_config config):
    Backend(sim, &(sim->simstate_)),
    config_(config)
{
    // Construct Testbench object
    tb = new Testbench<VAtomBones>();

    // Constuct Memory objects
    try
    {
        mem_["imem"] = std::shared_ptr<Memory> (new Memory(config_.imem_size_kb * 1024, config_.imem_offset, true));
        mem_["dmem"] = std::shared_ptr<Memory> (new Memory(config_.dmem_size_kb * 1024, config_.dmem_offset, false));
        mem_["pmem"] = std::shared_ptr<Memory> (new Memory(1024, 0x08000000, false));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    
    // ====== Initialize ========
    // Initialize memory
    // load text section
    try
    {
        mem_["imem"]->set_write_protect(false);
        init_from_elf(mem_["imem"].get(), sim_->sim_config_.ifile, std::vector<int>{5, 6});
        mem_["imem"]->set_write_protect(true);

        init_from_elf(mem_["dmem"].get(), sim_->sim_config_.ifile, std::vector<int>{5, 6});
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
       

    // Initialize CPU state by resetting
    this->reset();
    tb->m_core->eval();
    
    // get initial signal values
    this->refresh_state();

    // ====== Initialize Communication ========

    // create a new vuart object
    if(config_.using_vuart)
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
    if (config_.using_vuart)
    {
        // destroy vuart object
        if(vuart_ != nullptr)
            delete vuart_;
    }

    // delete tb;
    mem_.clear();
}

void Backend_atomsim::service_mem_req()
{    
    // Clear all ack signals
    tb->m_core->imem_ack_i = 0;
    tb->m_core->dmem_ack_i = 0;

    // ===== Imem Port Reads =====
    uint32_t iaddr = tb->m_core->imem_addr_o & 0xfffffffc;
    if(tb->m_core->imem_valid_o == 1)
    {   
        Word_alias instr_w;
        mem_["imem"]->fetch(iaddr, instr_w.byte, 4);
        tb->m_core->imem_data_i = instr_w.word;
        tb->m_core->imem_ack_i = 1;
    }

    // ===== Dmem Port Reads/Writes =====
    uint32_t daddr = tb->m_core->dmem_addr_o & 0xfffffffc;
    if(tb->m_core->dmem_valid_o)
    {
        bool success = false;
        for (auto mem_block: mem_)
        {
            std::shared_ptr<Memory> m = mem_block.second;
            if( daddr >= m->get_base_addr() && daddr < m->get_base_addr() + m->get_size())
            {
                if(tb->m_core->dmem_we_o)	// Writes
                {
                    Word_alias data_w = {.word = tb->m_core->dmem_data_o};

                    if(tb->m_core->dmem_sel_o & 0b0001) 
                        m->store(daddr, &data_w.byte[0], 1);
                    if(tb->m_core->dmem_sel_o & 0b0010) 
                        m->store(daddr, &data_w.byte[1], 1);
                    if(tb->m_core->dmem_sel_o & 0b0100) 
                        m->store(daddr, &data_w.byte[2], 1);
                    if(tb->m_core->dmem_sel_o & 0b1000) 
                        m->store(daddr, &data_w.byte[3], 1);
                }
                else    // Reads
                {
                    // Read will always result in a fetch word at word boundry just before the address.
                    Word_alias data_w;
                    m->fetch(daddr, data_w.byte, 4);
                    tb->m_core->dmem_data_i = data_w.word;
                }
                tb->m_core->dmem_ack_i = 1;

                success = true;
                break;  // exit search loop
            }
        }

        if (!success)
        {
            throw Atomsim_exception("Service memory request failed: address not in range of any memory block");
        }
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
        if(i==0)
            simstate_->state_.rf[i] = 0;
        else
            simstate_->state_.rf[i] = tb->m_core->AtomBones->atom_core->rf->regs[i-1];
    }

    // Get Signals
    simstate_->signals_.jump_decision = tb->m_core->AtomBones->atom_core->__PVT__jump_decision;

    // get Tickcounts
    simstate_->state_.tickcount = tb->m_tickcount;
    simstate_->state_.tickcount_total = tb->m_tickcount_total;
}


void Backend_atomsim::UART()
{
    // --------- Atom->port -----------
    /*
        Since in classical single wishbone write transaction, wb_we pin remains asserted until 
        the	tansaction is marked finish by the slave by setting the wb_ack pin. From prespective
        of UART, it sees the we pin high for multiple cycles and it may mistakenly infer it
        as multiple rreads/writes to same addrress. This piece of logic is to prevent that.
        When we fine 'we' asserted, we read the value and wait for 2 cycles after it. This 
        prevents multiple reads of data in same transaction.
    */		
    static int wait = 0;
    if(wait==0 && tb->m_core->dmem_valid_o && tb->m_core->dmem_we_o && tb->m_core->dmem_addr_o==0x08000000 && tb->m_core->dmem_sel_o==0b0001)
    {
        char c = (char)tb->m_core->dmem_data_o;

        if (config_.using_vuart)
            vuart_->send(c);
        else
            std::cout << c;

        wait=2;	// Wait for 2 cycles
    }
    
    try
    {
        /* code */
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    
    if(wait>0)
    {
        wait--;
    }

    // --------- Port->atom -----------
    /*
        This section of code deals with port to sim uart communication. port->recieve() is 
        called in every sim cycle (sim tick). value of recieve buffer is stored in 'recv' 
        variable. Now, if (recv!=-1) i.e. a valid character is present, it is stored in the 
        dummy hardware register of simpluart_wb, and set bit[0] of status register.
    */
    static uint8_t recv;
    if(config_.using_vuart)
    {
        recv = vuart_->recieve();
    }
    else
    {
        recv = (uint8_t)-1;
    }	
            
    if(recv != (uint8_t)-1)	// something recieved
    {
        uint8_t w[2] = {0x01, recv};                        // TODO: use word alias here
        mem_["pmem"]->store(0x08000000, w, 2);
    }

    /*
        If atom core tries to read data register, clear the data register (put -1) and clear 
        status bit[0].
    */
    if(tb->m_core->dmem_valid_o && ~tb->m_core->dmem_we_o && tb->m_core->dmem_addr_o==0x08000000 && tb->m_core->dmem_sel_o==0b0001)
    {
        uint8_t w[2] = {0x00, (uint8_t)-1};
        mem_["pmem"]->store(0x08000000, w, 2);
    }
}


int Backend_atomsim::tick()
{
    if(this->done())
    {
        return 1;
    }

    // Service Memory Request
    service_mem_req();

    // Tick clock once
    tb->tick();

    // Refresh Data
    if(sim_->sim_config_.debug_flag || sim_->sim_config_.dump_on_ebreak_flag)
    {
        refresh_state();
    }

    if(sim_->sim_config_.debug_flag)
        sim_->display_dbg_screen();


    // ===== Check Hault Condition =====
    if (tb->m_core->AtomBones->atom_core->InstructionRegister == RV_INSTR_EBREAK)
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


            fWrite(fcontents, sim_->sim_config_.dump_file.length() != 0 ? sim_->sim_config_.dump_file : "dump.log");
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
                mem_["dmem"]->fetch(addr, w.byte, 4);

                char temp [50];
                sprintf(temp, "%08x", w.word);
                fcontents.push_back(temp);
            }
            fWrite(fcontents, sim_->sim_config_.signature_file);
        }

        if (sim_->sim_config_.verbose_flag)
            std::cout << "Haulting @ tick " << tb->m_tickcount_total;
        return 1;
    }
    if(tb->m_tickcount_total > sim_->sim_config_.maxitr)
    {
        throwWarning("SIM0", "Simulation iterations exceeded maxitr("+std::to_string(sim_->sim_config_.maxitr)+")\n");
        return 1;
    }		

    // Serial port Emulator: Rx Listener
    UART();

    return 0;
}

void Backend_atomsim::fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)
{
    bool success = false;
    for (auto mem_block: mem_)  // search for mem blk
    {
        std::shared_ptr<Memory> m = mem_block.second;
        if( start_addr >= m->get_base_addr() && start_addr < m->get_base_addr() + m->get_size())
        {
            if(start_addr+buf_sz > m->get_base_addr() + m->get_size())
                throwError("", "cant fetch; bufsize too large for mem", false);
            
            m->fetch(start_addr, buf, buf_sz);
            success = true;
            break;  // exit search loop
        }
    }

    if (!success)
    {
        throw Atomsim_exception("memory fetch failed: no mem block at given address");
    }
}
    
void Backend_atomsim::store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz)
{
    bool success = false;
    for (auto mem_block: mem_)  // search for mem blk
    {
        std::shared_ptr<Memory> m = mem_block.second;
        if( start_addr >= m->get_base_addr() && start_addr < m->get_base_addr() + m->get_size())
        {
            if(start_addr+buf_sz > m->get_base_addr() + m->get_size())
                throwError("", "cant store; bufsize too large for mem", false);
            
            m->store(start_addr, buf, buf_sz);
            success = true;
            break;  // exit search loop
        }
    }

    if (!success)
    {
        throw Atomsim_exception("memory fetch failed: no mem block at given address");
    }
}