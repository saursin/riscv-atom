#include "Backend.hpp"

#include "include/elfio/elfio.hpp"

#include "verilated.h"
#include <verilated_vcd_c.h>

#include "../build/vobj_dir/VAtomBones.h"
#include "../build/vobj_dir/VAtomBones_AtomBones.h"
#include "../build/vobj_dir/VAtomBones_AtomRV.h"
#include "../build/vobj_dir/VAtomBones_RegisterFile__R20_RB5.h"

// Configuration
const unsigned int default_UART_RX_ADDRESS   	=	0x08000000;
const unsigned int default_UART_TX_ADDRESS      =	0x08000001;
const unsigned int default_UART_SREG_ADDRESS    =	0x08000002;

const unsigned int hault_opcode = 0x100073; // ebreak

/**
 * @brief Memory class
 * This class is used to emulate the memories in simulation backend
 * 
 */
class Memory
{
	public:
	/**
	 * @brief pointer to memory array
	 * 
	 */
	uint8_t * mem;

	/**
	 * @brief size of memory
	 * 
	 */
	uint32_t size;


	/**
	 * @brief Construct a new Memory object
	 * 
	 * @param max_addr size of memory
	 */
	Memory(uint32_t max_addr)
	{
		size = max_addr;

		// Allocate memory
		if(!(mem = new uint8_t[max_addr])) 
		{
			throwError("MEM0", "out of memory; memory allocation failed\n", true);
		}
	}


	/**
	 * @brief Destroy the Memory object
	 * 
	 */
	~Memory()
	{
		delete [] mem;
		size = 0;
	}


	/**
	 * @brief Check if the address is valid
	 * 
	 * @param addr address
	 * @return true if address is within bounds
	 * @return false if address is out of bounds
	 */
	bool isValidAddress(uint32_t addr)
	{
		return (addr < size);
	}


	/**
	 * @brief Fetch a 32-bit word from memory
	 * 
	 * @param addr address
	 * @return uint32_t data
	 */
	uint32_t fetchWord(uint32_t addr)
	{
		uint32_t byte0 = (uint32_t)fetchByte(addr);
		uint32_t byte1 = (uint32_t)fetchByte(addr+1);
		uint32_t byte2 = (uint32_t)fetchByte(addr+2);
		uint32_t byte3 = (uint32_t)fetchByte(addr+3);

		return ((byte3<<24) & 0xff000000) | ((byte2<<16) & 0x00ff0000) | ((byte1<<8) & 0x0000ff00) | (byte0 & 0x000000ff);
	}


	/**
	 * @brief Fetch a 16-bit half word from memory
	 * 
	 * @param addr address
	 * @return uint16_t data
	 */
	uint16_t fetchHalfWord(uint32_t addr)
	{
		uint32_t byte0 = (uint32_t)fetchByte(addr);
		uint32_t byte1 = (uint32_t)fetchByte(addr+1);

		return ((byte1<<8) & 0xff00) | (byte0 & 0x00ff);
	}


	/**
	 * @brief Fetch a 8-bitbyte word from memory
	 * 
	 * @param addr address
	 * @return uint8_t data
	 */
	uint8_t fetchByte(uint32_t addr)
	{
		if(!isValidAddress(addr))
		{
			char errmsg[40];
			sprintf(errmsg, "Address out of bounds : 0x%08x", addr);
			throwError("MEM1", errmsg, true);
			return 0;
		}
		return (uint8_t) mem[addr];
	}


	/**
	 * @brief Store a 32-bit word to memory
	 * 
	 * @param addr address
	 * @param w Word
	 */
	void storeWord(uint32_t addr, uint32_t w)
	{
		storeByte(addr,   (uint8_t) (w & 0x000000ff));
		storeByte(addr+1, (uint8_t) ((w & 0x0000ff00) >> 8));
		storeByte(addr+2, (uint8_t) ((w & 0x00ff0000) >> 16));
		storeByte(addr+3, (uint8_t) ((w & 0xff000000) >> 24));
	}


	/**
	 * @brief Store a 16-bit half word from memory
	 * 
	 * @param addr address
	 * @param hw halfWord
	 */
	void storeHalfWord(uint32_t addr, uint16_t hw)
	{
		storeByte(addr, (uint8_t) (hw & 0x00ff));
		storeByte(addr+1, (uint8_t) ((hw & 0xff00) >> 8));
	}


	/**
	 * @brief Store a 8-bit byte word from memory
	 * 
	 * @param addr address
	 * @param byte byte
	 */
	void storeByte(uint32_t addr, uint8_t byte)
	{
		if(!isValidAddress(addr))
		{
			char errmsg[40];
			sprintf(errmsg, "Address out of bounds : 0x%08x", addr);
			throwError("MEM1", errmsg, true);
			return;
		}
		mem[addr] = byte;
	}


	/**
	 * @brief Initialize memory from an elf file
	 * only sections that match flag signatures are loaded
	 * 
	 * @param ifile filename
	 * @param flags_signatures allowed flag signatures
	 */
	unsigned int initFromElf(std::string ifile, std::vector<int> flags_signatures)
	{
		// Initialize Memory object from input ELF File
		ELFIO::elfio reader;

		// Load file into elf reader
		if (!reader.load(ifile)) 
		{
			throwError("INIT0", "Can't find or process ELF file : " + ifile + "\n", true);
		}

		// Check ELF Class, Endiness & segment count
		if(reader.get_class() != ELFCLASS32)
			throwError("INIT1", "Elf file format invalid: should be 32-bit elf\n", true);
		if(reader.get_encoding() != ELFDATA2LSB)
			throwError("INIT2", "Elf file format invalid: should be little Endian\n", true);

		ELFIO::Elf_Half seg_num = reader.segments.size();

		if(seg_num == 0)
			throwError("INIT3", "Elf file format invalid: should consist of atleast one section\n", true);


		// Read elf and initialize memory
		if(verbose_flag)
			std::cout << "Segments found : "<< seg_num <<"\n";

		unsigned int i = 0;
		while (i < seg_num) // iterate over all segments
		{
			const ELFIO::segment * seg = reader.segments[i];

			// Get segment properties
			
			if (seg->get_type() == SHT_PROGBITS)
			{
				int seg_flags = reader.segments[i]->get_flags();

				if(flags_signatures.end() != std::find(flags_signatures.begin(), flags_signatures.end(), seg_flags))	// Flag found in signature list
				{

					const char* seg_data = reader.segments[i]->get_data();
					const uint seg_size = reader.segments[i]->get_file_size();
					ELFIO::Elf64_Addr seg_strt_addr = reader.segments[i]->get_physical_address();

					if(verbose_flag)
						printf("Loading Segment %d @ 0x%08x --- ", i, (unsigned int) reader.segments[i]->get_physical_address());
					
					long unsigned int offset = 0;
					while(offset<seg_size)
					{
						storeByte(seg_strt_addr + offset, seg_data[offset]);
						offset++;
					}

					if(verbose_flag)
						printf("done\n");
				}
			}
			i++;
		}
		return (unsigned int) reader.get_entry();
	}
};


/**
 * @brief Backend class
 * Backend class encapsulates the data 
 * probing and printing operations
 */
class Backend_AtomSim: public Backend<VAtomBones>
{
	public:
	/**
	 * @brief Pointer to memory object
	 */
	Memory * mem;


	/**
	 * @brief Construct a new Backend object
	 */
	Backend_AtomSim(std::string ifile, unsigned long mem_size)
	{
        // Construct Testbench object
        tb = new Testbench<VAtomBones>();

		// Constuct Memory object
		mem = new Memory(mem_size);

		// get input file disassembly
		disassembly = getDisassembly(ifile);

		// ====== Initialize ========
		// Initialize memory
		mem->initFromElf(ifile, std::vector<int>{5, 6}); // load text & data sections

		// Initialize CPU state by resetting
		reset();
		tb->m_core->eval();

        // get initial signal values
        refreshData();

		if (verbose_flag)
			std::cout << "Initialization complete!\n";
	}


	/**
	 * @brief Destroy the Backend object
	 */
	~Backend_AtomSim()
	{
		delete tb;
		delete mem;
	}

	std::string getTargetName()
	{
		return "AtomBones";
	}

	void serviceMemoryRequest()
	{
		// Clear all ack signals
		tb->m_core->imem_ack_i = 0;
		tb->m_core->dmem_ack_i = 0;

		// ===== Imem Port Reads =====
		uint32_t iaddr = tb->m_core->imem_addr_o & 0xfffffffc;
		if(tb->m_core->imem_valid_o == 1)
		{
			tb->m_core->imem_data_i = mem->fetchWord(iaddr);
			tb->m_core->imem_ack_i = 1;
		}

		// ===== Dmem Port Reads/Writes =====
		uint32_t daddr = tb->m_core->dmem_addr_o & 0xfffffffc;
		if(tb->m_core->dmem_valid_o)
		{
			if(tb->m_core->dmem_we_o)	// Writes
			{
				// Writes are selective
				if(tb->m_core->dmem_sel_o & 0b0001)
					mem->storeWord(daddr, (mem->fetchWord(daddr) & 0xffffff00) | (tb->m_core->dmem_data_o & 0x000000ff));
				if(tb->m_core->dmem_sel_o & 0b0010)
					mem->storeWord(daddr, (mem->fetchWord(daddr) & 0xffff00ff) | (tb->m_core->dmem_data_o & 0x0000ff00));
				if(tb->m_core->dmem_sel_o & 0b0100)
					mem->storeWord(daddr, (mem->fetchWord(daddr) & 0xff00ffff) | (tb->m_core->dmem_data_o & 0x00ff0000));
				if(tb->m_core->dmem_sel_o & 0b1000)
					mem->storeWord(daddr, (mem->fetchWord(daddr) & 0x00ffffff) | (tb->m_core->dmem_data_o & 0xff000000));		
			}
			else	// Reads
			{
				// Read will always result in a fetch word at word boundry just before the address.
				tb->m_core->dmem_data_i = mem->fetchWord(daddr);
			}
			tb->m_core->dmem_ack_i = 1;
		}
	}


	/**
	 * @brief probe all internal signals and registers and 
	 * update backend state
	 */
	void refreshData()
	{
		state.pc_f = tb->m_core->AtomBones->atom_core->ProgramCounter;
		state.pc_e = tb->m_core->AtomBones->atom_core->ProgramCounter_Old;

		state.ins_e = tb->m_core->AtomBones->atom_core->InstructionRegister;
		for(int i=0; i<32; i++)
		{
			if(i==0)
				state.rf[i] = 0;
			else
				state.rf[i] = tb->m_core->AtomBones->atom_core->rf->regs[i-1];
		}

		signals.jump_decision = tb->m_core->AtomBones->atom_core->__PVT__jump_decision;
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

		// Service Memory Request
		serviceMemoryRequest();

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
		if (tb->m_core->AtomBones->atom_core->InstructionRegister == hault_opcode)
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
					throwError("SIG", "One or both of 'begin_signature' & 'end_signature' symbols missing from ELF symbol table: " + ifile + "\n", true);
				}

				//  dump data to signature file
				std::vector<std::string> fcontents;
				printf("Dumping signature region [0x%08lx-0x%08lx]\n", begin_signature_at, end_signature_at);

				for(long int i=begin_signature_at; i<end_signature_at; i=i+4)
				{
					char temp [50];
					sprintf(temp, "%08x", mem->fetchWord(i));
					fcontents.push_back(temp);
				}
				fWrite(fcontents, signature_file);
			}

			if (verbose_flag)
				std::cout << "Haulting @ tick " << tb->m_tickcount_total;
			ExitAtomSim("\n");
		}
		if(tb->m_tickcount_total > maxitr)
		{
			throwWarning("SIM0", "Simulation iterations exceeded maxitr("+std::to_string(maxitr)+")\n");
			ExitAtomSim("");
		}		

		// Serial port Emulator: Rx Listener
		if((tb->m_core->dmem_valid_o) && (tb->m_core->dmem_addr_o==0x08000000) 
			&& (tb->m_core->dmem_sel_o == 0b010))	// Write to TX reg
		{
			uint32_t data = tb->m_core->dmem_data_o;
			data = (data & 0x0000ff00) >> 8;
			std::cout << (char)data;
		}

	}

	/**
	 * @brief Dump contents of memoy into a file
	 * @param file 
	 */
	void dumpmem(std::string file)
	{
		if(mem->size < 1*1024*1024)	// 1MB
		{
			std::vector<std::string> fcontents;
			for(unsigned int i=0; i<mem->size-4; i+=4)
			{	
				char hex [30];
				sprintf(hex, "0x%08x\t:\t0x%08x", i, mem->fetchWord(i));
				fcontents.push_back(hex);
			}
			fWrite(fcontents, std::string(default_dump_dir)+"/"+file);
		}
		else
			throwError("","Option not available due to excessive memory size (>1MB)\n", false);
	}

	 /**
     * @brief Get contents of a memory location
     */
    uint32_t getMemContents(uint32_t addr)
    {
        return mem->fetchWord(addr);
    }
};
