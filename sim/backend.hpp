#include "../lib/elfio/elfio.hpp"


#include "verilated.h"
#include <verilated_vcd_c.h>
#include "../build/obj_dir/VAtomRVSoC.h"
#include "../build/obj_dir/VAtomRVSoC_AtomRVSoC.h"
#include "../build/obj_dir/VAtomRVSoC_AtomRV.h"
#include "../build/obj_dir/VAtomRVSoC_RegisterFile__R20_RB5.h"

/**
 * @brief TESTBENCH Class
 * 
 * @tparam VTop module to be instantiated
 */
template <class VTop>
class TESTBENCH 
{
	public:

	VTop 			* m_core;
	VerilatedVcdC	* m_trace;
	unsigned long 	m_tickcount;     			// TickCounter to count clock cycles fom last reset
	unsigned long 	m_tickcount_total;   		// TickCounter to count clock cycles

	/**
	 * @brief Construct a new TESTBENCH object
	 * 
	 */
	TESTBENCH(void)                // Constructor: Instantiates a new VTop
    {
		m_core = new VTop;
		Verilated::traceEverOn(true);
		m_tickcount = 0l;
	}


	/**
	 * @brief Destroy the TESTBENCH object
	 * 
	 */
	virtual ~TESTBENCH(void)       // Destructor 
    {
		delete m_core;
		m_core = NULL;
	}


	/**
	 * @brief Open/create a trace file
	 * 
	 * @param vcdname name of vcd file
	 */
	virtual	void openTrace(const char *vcdname) 
	{
		if (!m_trace) 
		{
			m_trace = new VerilatedVcdC;
			m_core->trace(m_trace, 99);
			m_trace->open(vcdname);
		}
	}


	/**
	 * @brief Close a trace file
	 * 
	 */
	virtual void closeTrace(void) 
	{
		if (m_trace) 
		{
			m_trace->close();
			m_trace = NULL;
		}
	}


	/**
	 * @brief Reset topmodule
	 * 
	 */
	virtual void reset(void) 
    {
		m_core-> rst_i = 1;
		this -> tick();	// Make sure any inheritance gets applied
		m_tickcount = 0;
		m_core-> rst_i = 0;
	}


	/**
	 * @brief Run for one cycle
	 * 
	 */
	virtual void tick(void) 
    {
		// Increment our own internal time reference
		m_tickcount++;
		m_tickcount_total++;

        // Make sure any combinatorial logic depending upon
		// inputs that may have changed before we called tick()
		// has settled before the rising edge of the clock.
		m_core -> clk_i = 0;
		m_core -> eval();

		//	Dump values to our trace file before clock edge
		if(m_trace) m_trace->dump(10*m_tickcount-2);

		// ---------- Toggle the clock ------------

		// Rising edge
		m_core -> clk_i = 1;
		m_core -> eval();

		//	Dump values to our trace file after clock edge
		if(m_trace) m_trace->dump(10*m_tickcount);


		// Falling edge
		m_core -> clk_i = 0;
		m_core -> eval();


		if (m_trace) {
			// This portion, though, is a touch different.
			// After dumping our values as they exist on the
			// negative clock edge ...
			m_trace->dump(10*m_tickcount+5);
			//
			// We'll also need to make sure we flush any I/O to
			// the trace file, so that we can use the assert()
			// function between now and the next tick if we want to.
			m_trace->flush();
		}
	}


	/**
	 * @brief Check if simulation ended
	 * 
	 * @return true if verilator has encountered $finish
	 * @return false if verilator hasn't encountered $finish yet
	 */
	virtual bool  done(void)
	{
		return (Verilated::gotFinish()); 
	}
};


/**
 * @brief Memory class
 * This class is used to emulate the memories in simulation backend
 * 
 */
class Memory
{
	public:
	uint8_t * mem;
	uint32_t size;

	/**
	 * @brief Construct a new Memory object
	 * 
	 * @param max_addr size of memory
	 * @param filename name of init file
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
		return (addr < size-4);
	}

	/**
	 * @brief Fetch a 32-bit word from memory
	 * 
	 * @param addr address
	 * @return uint32_t data
	 */
	uint32_t fetchWord(uint32_t addr)
	{
		uint32_t data;
		if(!isValidAddress(addr))
			throwError("MEM1", "Address out of bounds " + std::to_string(addr) + "\n", true);

		uint32_t byte0 = (uint32_t)mem[addr];
		uint32_t byte1 = (uint32_t)mem[addr+1];
		uint32_t byte2 = (uint32_t)mem[addr+2];
		uint32_t byte3 = (uint32_t)mem[addr+3];

		return (byte3<<24 & 0xff000000) | (byte2<<16 & 0x00ff0000) |(byte1<<8 & 0x0000ff00) | (byte0 & 0x000000ff);
	}

	/**
	 * @brief Fetch a 8-bitbyte word from memory
	 * 
	 * @param addr address
	 * @return uint32_t data
	 */
	uint8_t fetchByte(uint32_t addr)
	{
		uint32_t data;
		if(!isValidAddress(addr))
			throwError("MEM1", "Address out of bounds " + std::to_string(addr) + "\n", true);
		
		return (uint32_t)mem[addr];
	}

	/**
	 * @brief Store a 8-bit byte word from memory
	 * 
	 * @param addr address
	 * @param byte byte
	 */
	void storeByte(uint32_t addr, uint8_t byte)
	{
		uint32_t data;
		if(!isValidAddress(addr))
			throwError("MEM1", "Address out of bounds " + std::to_string(addr) + "\n", true);
		else
		{
			mem[addr] = byte;
		}
	}
};



/**
 * @brief Initialize IMEM & DMEM fom an elf file
 * 
 * @param imem pointer to imem object
 * @param dmem pointer to dmem object
 * @param ifile elf file name
 * @throws char *
 */
void initMem(Memory * imem, Memory * dmem, std::string ifile)
{
    // Initialize Memory object from input ELF File
    ELFIO::elfio reader;

    // Load file into elf reader
    if (!reader.load(ifile)) 
    {
        throwError("INIT0", "Can't find or process ELF file : " + ifile + "\n", true);
    }

    // Check ELF Class, Endiness & segment count
    if(reader.get_class() == ELFCLASS32)
		throwError("INIT1", "Elf file format invalid: should be 32-bit elf\n", true);
    if(reader.get_encoding() == ELFDATA2LSB)
		throwError("INIT2", "Elf file format invalid: should be little Endian\n", true);
    if(reader.segments.size() < 2)
		throwError("INIT3", "Elf file format invalid: should consist of atleast two sections\n", true);

    ELFIO::Elf_Half seg_num = reader.segments.size();

    // Read elf and initialize memory
    bool imem_done = false;
    bool dmem_done = false;

    for (int i = 0; i < seg_num; ++i) // iterate over all segments
    {
        const ELFIO::segment * pseg = reader.segments[i];

        // Access segments's data
        if(pseg->get_type() == SHT_PROGBITS)
        {
            const char* p = reader.segments[i]->get_data();
            const uint sz = reader.segments[i]->get_file_size();

            std::string segType = "UNKNOWN";
            if(reader.segments[i]->get_flags() == 5) // Read & Execute Pemission =>> TEXT Segment
            {
                segType = "TEXT";
                imem_done = true;
            }
            else if(reader.segments[i]->get_flags() == 6) // Read & Write Pemission =>> DATA Segment
            {
                segType = "DATA";
                imem_done = true;
            }


            // Initialize
            unsigned int i = 0;
            while(i<sz)
            {
                if(segType == "TEXT")       // initialize imem
                {
                    imem->storeByte(i, p[i]);
                }
                else if (segType == "DATA") // initialize dmem
                {
                    dmem->storeByte(i, p[i]);
                }
                i++;
            }
        }
    }
}

/**
 * @brief Backend class
 * Backend class encapsulates the data 
 * probing and printing operations
 */
class Backend
{
	public:
	// Testbench
	TESTBENCH<VAtomRVSoC> *tb;

	// Instruction memory
	Memory * imem;

	// ==== STATE ====
	enum CpuState{FETCH, EXECUTE};
	CpuState cpu_state;

	unsigned int pc;
	unsigned int rf[32];

	unsigned int ir;
	
	/**
	 * @brief Construct a new Backend object
	 * 
	 * @param imem_init_file init_file
	 */
	Backend(std::string imem_init_file)
	{
		tb = new TESTBENCH<VAtomRVSoC>();
		imem = new Memory(65536);
		refreshData();
	}


	/**
	 * @brief Destroy the Backend object
	 */
	~Backend()
	{
		delete tb;
		delete imem;
	}


	/**
	 * @brief reset the backend
	 */
	void reset()
	{
		tb->reset();
	}


	/**
	 * @brief probe all internal signals and regsters and 
	 * update backend state
	 */
	void refreshData()
	{
		cpu_state = (CpuState)tb->m_core->AtomRVSoC->atom->ProcessorState;
		pc = tb->m_core->AtomRVSoC->atom->ProgramCounter;
		ir = tb->m_core->AtomRVSoC->atom->InstructionRegister;
		for(int i=0; i<32; i++)
			rf[i] = tb->m_core->AtomRVSoC->atom->rf->regs[i];
	}


	/**
	 * @brief Display state data on console
	 */
	void displayData()
	{
		/*if(cpu_state == FETCH)
		{
			std::cout << "\nF-< "<<tb->m_tickcount<<" >\n";
			return;
		}*/
		std::cout << "E-< " << tb->m_tickcount <<" >---------------------------------------------------------------\n";

		printf(" pc : 0x%08X   ir : 0x%08X   []\n\n", pc , ir); 

		if(verbose_flag)
		{
			int cols = 4; // no of coloumns per rows
			/*for(int i=0; i<32; i++)	// print in left-right fashion
			{
				printf("r%-2d: 0x%08X   ", i, rf[i]);
				if(i%cols == cols-1)
					printf("\n");
			}*/
			for(int i=0; i<32/cols; i++)	// print in topdown fashion
			{
				for(int j=0; j<cols; j++)
				{
					printf(" r%-2d: 0x%08X  ", i+(32/cols)*j, rf[i+(32/cols)*j]);
				}
				printf("\n");
			}
		}
	}


	/**
	 * @brief Tick for one cycle
	 * 
	 */
	void tick()
	{
		tb->m_core->imem_data_i = imem->fetchWord(tb->m_core->imem_addr_o);
		tb->tick();
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




/*void runBackend()
{
	while(true)
	{
		if(backend_finished || frontend_finished)
				return;

		if(backend_run | backend_tick)
		{
			if (backend_tick)	// uns fo this current cycle only and turns backend tick as false, backennd_tick needs to be settue by frontend to again execute next cycle
				backend_tick = false;

			backend_finished = tb->done();

			// Registers
			d.PC = tb->m_core->Top->luna_soc->CPU->PC;
			
			// Signals
			d.Signals[0] = tb->m_core->Top->luna_soc->CPU->D_Opcode;
			d.Signals[1] = tb->m_core->Top->luna_soc->CPU->D_Func;
			d.Signals[2] = tb->m_core->Top->luna_soc->CPU->D_Rd_Sel;
			d.Signals[3] = tb->m_core->Top->luna_soc->CPU->D_Rs_Sel;
			d.Signals[4] = tb->m_core->Top->luna_soc->CPU->D_Imm;
			d.Signals[5] = tb->m_core->Top->luna_soc->CPU->D_BrReg;
			d.Signals[6] = tb->m_core->Top->luna_soc->CPU->D_BrLink;

			for(int i=0; i<16; i++)
			{
				d.REG[i] = tb->m_core->Top->luna_soc->CPU->rf->regs[i];
			}
			

			

			usleep(backend_delay);
		}
	}
}
*/