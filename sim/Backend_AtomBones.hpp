#include "include/elfio/elfio.hpp"

#include "verilated.h"
#include <verilated_vcd_c.h>
#include "../build/vobj_dir/VAtomBones.h"
#include "../build/vobj_dir/VAtomBones_AtomBones.h"
#include "../build/vobj_dir/VAtomBones_AtomRV.h"
#include "../build/vobj_dir/VAtomBones_RegisterFile__R20_RB5.h"

#include "Testbench.hpp"

const unsigned int default_UART_RX_ADDRESS   	=	0x08000000;
const unsigned int default_UART_TX_ADDRESS      =	0x08000001;
const unsigned int default_UART_SREG_ADDRESS    =	0x08000002;



/**
 * @brief Register ABI names used in debug display 
 * 
 */
const std::vector<std::string> reg_names = 
{
	"x0  (zero) ",
	"x1  (ra)   ",
	"x2  (sp)   ",
	"x3  (gp)   ",
	"x4  (tp)   ",
	"x5  (t0)   ",
	"x6  (t1)   ",
	"x7  (t2)   ",
	"x8  (s0/fp)",
	"x9  (s1)   ",
	"x10 (a0)   ",
	"x11 (a1)   ",
	"x12 (a2)   ",
	"x13 (a3)   ",
	"x14 (a4)   ",
	"x15 (a5)   ",
	"x16 (a6)   ",
	"x17 (a7)   ",
	"x18 (s2)   ",
	"x19 (s3)   ",
	"x20 (s4)   ",
	"x21 (s5)   ",
	"x22 (s6)   ",
	"x23 (s7)   ",
	"x24 (s8)   ",
	"x25 (s9)   ",
	"x26 (s10)  ",
	"x27 (s11)  ",
	"x28 (t3)   ",
	"x29 (t4)   ",
	"x30 (t5)   ",
	"x31 (t6)   "
};

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
class Backend
{
	public:
	/**
	 * @brief Pointer to testbench object
	 */
	Testbench<VAtomBones> *tb;

	// memory
	/**
	 * @brief Pointer to memoy object
	 */
	Memory * mem;

	// ==== STATE ====
	/**
	 * @brief Program counter value in fetch stage
	 */
	unsigned int pc_f;

	/**
	 * @brief Instruction in fetch stage
	 */
	unsigned int ins_f;

	/**
	 * @brief Program counter value in execute stage
	 */
	unsigned int pc_e;

	/**
	 * @brief Instuction value in execute stage
	 */
	unsigned int ins_e;
	
	/**
	 * @brief Registe file values
	 */
	unsigned int rf[32];

	/**
	 * @brief Disassembly of input file
	 */
	std::map<uint32_t, std::string> disassembly;

	/**
	 * @brief Construct a new Backend object
	 * 
	 * @param mem_init_file init_file
	 */
	Backend(std::string mem_init_file, unsigned int mem_size)
	{
		tb = new Testbench<VAtomBones>();
		mem = new Memory(mem_size);

		unsigned int entry_point = mem->initFromElf(mem_init_file, std::vector<int>{5, 6}); // load text & data sections
		if (verbose_flag)
			printf("Entry point : 0x%08x\n", entry_point);

		// Set entry point
		tb->m_core->AtomBones->atom_core->ProgramCounter = entry_point;

		tb->m_core->eval();

		// get initial signal values
		refreshData();

		// get input file disassembly
		disassembly = getDisassembly(mem_init_file);
		
		if (verbose_flag)
			std::cout << "Initialization complete!\n";
	}


	/**
	 * @brief Destroy the Backend object
	 */
	~Backend()
	{
		delete tb;
		delete mem;
	}


	/**
	 * @brief reset the backend
	 */
	void reset()
	{
		tb->reset();
	}


	void serviceMemoryRequest()
	{
		// Clear all ack signals
		tb->m_core->imem_ack_i = 0;
		tb->m_core->dmem_ack_i = 0;

		// Imem Port Reads
		if(tb->m_core->imem_valid_o == 1)
		{	
			tb->m_core->imem_data_i = mem->fetchWord(tb->m_core->imem_addr_o);
			tb->m_core->imem_ack_i = 1;
		}

		// Dmem Port Reads/Writes
		if(tb->m_core->dmem_valid_o && !tb->m_core->dmem_we_o)	// Load instruction
		{
			tb->m_core->dmem_data_i = mem->fetchWord(tb->m_core->dmem_addr_o);
			tb->m_core->dmem_ack_i = 1;
		}
		else if(tb->m_core->dmem_valid_o && tb->m_core->dmem_we_o)	// Store instruction
		{
			switch(tb->m_core->dmem_sel_o)
			{
				case 0x1:	mem->storeByte(tb->m_core->dmem_addr_o, (uint8_t)tb->m_core->dmem_data_o);	break;
				case 0x3:	mem->storeHalfWord(tb->m_core->dmem_addr_o, (uint16_t)tb->m_core->dmem_data_o);	break;
				case 0xf:	mem->storeWord(tb->m_core->dmem_addr_o, (uint32_t)tb->m_core->dmem_data_o); break;
				default: 
					std::cout << std::hex << (int)tb->m_core->dmem_sel_o << std::endl;
					throwError("RTL", "signal 'dmem_sel_o' has unexpected value");

			}
			tb->m_core->dmem_ack_i = 1;
		}
	}


	/**
	 * @brief probe all internal signals and regsters and 
	 * update backend state
	 */
	void refreshData()
	{
		pc_f = tb->m_core->AtomBones->atom_core->ProgramCounter;
		pc_e = tb->m_core->AtomBones->atom_core->ProgramCounter_Old;

		ins_e = tb->m_core->AtomBones->atom_core->InstructionRegister;
		for(int i=0; i<32; i++)
		{
			if(i==0)
				rf[i] = 0;
			else
				rf[i] = tb->m_core->AtomBones->atom_core->rf->regs[i-1];
		}
	}


	/**
	 * @brief Display state data on console
	 */
	void displayData()
	{
		unsigned int change = pc_f-pc_e;
		std::string jump = "    ";
		bool isJump = tb->m_core->AtomBones->atom_core->__PVT__jump_decision;
		if(isJump)
			jump = "jump";
		else
			jump = "    ";
		static bool wasJump = false;
		std::cout << "-< " << tb->m_tickcount <<" >--------------------------------------------\n";
		printf("F-STAGE  |  pc : 0x%08x  (%+d) (%s) \n", pc_f , change, jump.c_str()); 
		printf("E-STAGE  V  pc : 0x%08x   ir : 0x%08x \n", pc_e , ins_e); 
		
		std::cout << "[ " <<  disassembly[pc_e] << " ] ";
		
		if(wasJump)
			std::cout << " => nop (pipeline flush)";

		std::cout << "\n";

		wasJump = isJump;

		std::cout << "---------------------------------------------------\n";
						
		if(verbose_flag)
		{
			int cols = 2; // no of coloumns per rows
			#ifndef DEBUG_PRINT_T2B
			for(int i=0; i<32; i++)	// print in left-right fashion
			{
				printf("r%-2d: 0x%08x   ", i, rf[i]);
				if(i%cols == cols-1)
					printf("\n");
			}
			#else
			for(int i=0; i<32/cols; i++)	// print in topdown fashion
			{
				for(int j=0; j<cols; j++)
				{
					printf(" %s: 0x%08x  ", reg_names[i+(32/cols)*j].c_str(), rf[i+(32/cols)*j]);
				}
				printf("\n");
			}
			#endif
		}
	}


	/**
	 * @brief Tick for one cycle
	 * 
	 */
	void tick()
	{
		serviceMemoryRequest();
		tb->tick();
		ins_f = tb->m_core->imem_data_i;

		// Serial port Emulator: Rx Listener
		static bool prev_tx_we = false;
		bool cur_tx_we = (mem->fetchByte(default_UART_SREG_ADDRESS) & 1);
		if(prev_tx_we == false && cur_tx_we == true) // posedge on tx_we
		{
			std::cout << (char)mem->fetchByte(default_UART_TX_ADDRESS);
		}
		prev_tx_we = cur_tx_we;
	}


	/**
	 * @brief check if simulation is done
	 * 
	 * @return true 
	 * @return false 
	 */
	bool done()
	{
		return tb->done();
	}
};
