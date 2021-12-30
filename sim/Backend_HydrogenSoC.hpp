#include "Backend.hpp"

#include "include/elfio/elfio.hpp"

#include "verilated.h"
#include <verilated_vcd_c.h>

#include "../build/vobj_dir/VHydrogenSoC.h"
#include "../build/vobj_dir/VHydrogenSoC_HydrogenSoC.h"
#include "../build/vobj_dir/VHydrogenSoC_AtomRV_wb.h"
#include "../build/vobj_dir/VHydrogenSoC_AtomRV.h"
#include "../build/vobj_dir/VHydrogenSoC_RegisterFile__R20_RB5.h"

#include "../build/vobj_dir/VHydrogenSoC_DualPortRAM_wb__pi1.h"
#include "../build/vobj_dir/VHydrogenSoC_SinglePortRAM_wb__pi2.h"
#include "../build/vobj_dir/VHydrogenSoC_simpleuart_wb.h"

const unsigned int hault_opcode = 0x100073; // ebreak

/**
 * @brief Backend class
 * Backend class encapsulates the data 
 * probing and printing operations
 */
class Backend_AtomSim: public Backend<VHydrogenSoC>
{
	public:
	/**
	 * @brief Pointer to Vuart object
	 */
	Vuart *vuart = nullptr;

	/**
	 * @brief Construct a new Backend object
	 */
	Backend_AtomSim(std::string ifile)
	{
		// Convert ELF file to memory images
		std::string cmd_output = GetStdoutFromCommand("python3 $RVATOM/scripts/convelf.py "+ ifile);
		if(cmd_output.length() > 0)
		{
			throwWarning("ELF", "Failed to generate hexfiles");
			std::cerr << cmd_output << "";
			ExitAtomSim("", true);
		}

		// Construct Testbench object
		tb = new Testbench<VHydrogenSoC>();

		// get input file disassembly
		disassembly = getDisassembly(ifile);

		// ====== Initialize ========

		// Initialize CPU state by resetting
		reset();
		tb->m_core->eval();

		// get initial signal values
		refreshData();

		// ====== Initialize VUART ========

		// create a new vuart object
		if(vuart_portname != "Null")
		{	
			vuart = new Vuart(vuart_portname, vuart_baudrate);
				
			// Clean recieve buffer
			vuart->clean_recieve_buffer();

			if(verbose_flag)
				std::cout << "Connected to VUART ("+vuart_portname+") at "+std::to_string(vuart_baudrate)+" bps" << std::endl;
		}

		if (verbose_flag)
			std::cout << "Initialization complete!\n";
	}

	/**
	 * @brief Destroy the Backend object
	 */
	~Backend_AtomSim()
	{
		// destroy vuart object
		if(vuart != nullptr)
			delete vuart;

		delete tb;
	}

	std::string getTargetName()
	{
		return "HydrogenSoC";
	}

	/**
	 * @brief probe all internal signals and regsters and 
	 * update backend state
	 */
	void refreshData()
	{
		state.pc_f = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->ProgramCounter;
		state.pc_e = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->ProgramCounter_Old;

		state.ins_e = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->InstructionRegister;
		for(int i=0; i<32; i++)
		{
			if(i==0)
				state.rf[i] = 0;
			else
				state.rf[i] = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->rf->regs[i-1];
		}

		signals.jump_decision = tb->m_core->HydrogenSoC->atom_wb_core->atom_core->__PVT__jump_decision;
	}

	/**
	 * @brief Performs UART Transactions. 
	 */
	void UART()
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

		if(wait==0 && tb->m_core->HydrogenSoC->uart->reg_data_we)
		{
			char c = (char)tb->m_core->HydrogenSoC->uart->wb_dat_i;

			if (vuart_portname != "Null")
				vuart->send(c);
			else
				std::cout << c;

			wait=2;	// Wait for 2 cycles
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
		static char recv;
		recv = vuart->recieve();
				
		if(recv != (char)-1)	// something recieved
		{
			tb->m_core->HydrogenSoC->uart->reg_status = 1;
			tb->m_core->HydrogenSoC->uart->reg_data = recv;
		}

		/*
			If atom core tries to read data register, clear the data register (put -1) and clear 
			status bit[0].
		*/
		if(tb->m_core->HydrogenSoC->uart->reg_data_re)
		{
			tb->m_core->HydrogenSoC->uart->reg_status = 0;
			tb->m_core->HydrogenSoC->uart->reg_data = (char)-1;
		}
	}

	/**
	 * @brief Tick for one cycle
	 * 
	 */
	void tick()
	{
		//
		if(done())
		{
			ExitAtomSim("Encountered $finish()");
		}

		// Tick clock once
		tb->tick();

		// Refresh Data
		if(debug_mode || dump_regs_on_ebreak)
		{
			refreshData();
		}

		if(debug_mode)
			displayData();


		// ===== Check Hault Condition =====
		if (tb->m_core->HydrogenSoC->atom_wb_core->atom_core->InstructionRegister == hault_opcode)
		{
			// ============ REGISTER FILE DUMP (For SCAR) ==============
			if(dump_regs_on_ebreak)
			{
				std::vector<std::string> fcontents;

				for(int i=0; i<34; i++)
				{
					char temp [50];
					unsigned int tmpval;

					switch(i-2)
					{
						case -2: tmpval = state.pc_e; sprintf(temp, "pc 0x%08x", tmpval); break;
						case -1: tmpval = state.ins_e; sprintf(temp, "ir 0x%08x", tmpval); break;
						default: tmpval = state.rf[i-2]; sprintf(temp, "x%d 0x%08x",i-2, tmpval); break;
					}
					fcontents.push_back(std::string(temp));
				}
				fWrite(fcontents, std::string(trace_dir)+"/dump.txt");
			}

			// ==========  MEM SIGNATURE DUMP (For RISC-V-Arch Tests) =============
			if(signature_file.length()!=0)	
			{
				// Get start and end address of signature
				long int begin_signature_at = -1;
				long int end_signature_at = -1;

				ELFIO::elfio reader;

				if (!reader.load(ifile))	
				{
					throwError("SIG", "Can't find or process ELF file : " + ifile + "\n", true);
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
							for ( ELFIO::Elf_Xword i = 0; i < sym_no; ++i )
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

				if(begin_signature_at == -1 || end_signature_at == -1)
				{
					throwError("SIG", "One or both of 'begin_signature' & 'end_signature' symbols missing from ELF symbol table: " + ifile + "\n", true);
				}

				//  dump data to signature file
				std::vector<std::string> fcontents;
				if (verbose_flag)
					printf("Dumping signature region [0x%08lx-0x%08lx]\n", begin_signature_at, end_signature_at);

				for(long int i=begin_signature_at; i<end_signature_at; i=i+4)
				{
					int64_t addr = i-0x04000000;
					if (!((addr>0) && (addr <= 8*1024-4)))
						throwError("", "Signature Out of Bounds" + std::to_string(0x04000000+i), true);
					
					char temp [50];
					uint32_t value = (uint32_t)tb->m_core->HydrogenSoC->dmem->mem[addr/4];
					sprintf(temp, "%08x", value);
					fcontents.push_back(temp);
				}
				fWrite(fcontents, signature_file);
			}

			if (verbose_flag)
				std::cout << "Haulting @ tick " << tb->m_tickcount_total << "\n";
			ExitAtomSim("");
		}
		if(tb->m_tickcount_total > maxitr)
		{
			throwWarning("SIM0", "Simulation iterations exceeded maxitr("+std::to_string(maxitr)+")\n");
			ExitAtomSim("");
		}

		// Serial port Emulator
		UART();
	}

	void dumpmem(std::string file)
	{
		std::vector<std::string> fcontents;
		unsigned int base = 0;
		char line [50];
		sprintf(line, "######## IMEM (base 0x%08x) ########", base);
		fcontents.push_back(line);

		for(unsigned int i=0; i<8192; i++)
		{
			sprintf(line, "0x%08x: 0x%08x",i+base, tb->m_core->HydrogenSoC->imem->mem[i]);
			fcontents.push_back(line);
		}

		base = 0x04000000;
		sprintf(line, "\n\n\n######## DMEM (base 0x%08x) ########", base);
		fcontents.push_back(line);
		for(unsigned int i=0; i<2048; i++)
		{
			sprintf(line, "0x%08x: 0x%08x",i+base, tb->m_core->HydrogenSoC->dmem->mem[i]);
			fcontents.push_back(line);
		}
		fWrite(fcontents, std::string(default_dump_dir)+"/"+file);
	}
};
