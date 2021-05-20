#include "verilated.h"
#include "../build/obj_dir/VAtomRVSoC.h"
#include "../build/obj_dir/VAtomRVSoC_AtomRVSoC.h"
#include "../build/obj_dir/VAtomRVSoC_AtomRV.h"
#include "../build/obj_dir/VAtomRVSoC_RegisterFile__R20_RB5.h"

template <class VTop> 
class TESTBENCH 
{
	public:

	VTop * m_core;
	unsigned long tickcount;     		// TickCounter to count clock cycles fom last reset
	unsigned long tickcount_total;     	// TickCounter to count clock cycles

	TESTBENCH(void)                // Constructor: Instantiates a new VTop
    {
		m_core = new VTop;
		tickcount = 0l;
	}

	virtual ~TESTBENCH(void)       // Destructor 
    {
		delete m_core;
		m_core = NULL;
	}

	virtual void reset(void) 
    {
		m_core-> rst_i = 1;

		this -> tick();	// Make sure any inheritance gets applied
		tickcount = 0;
		m_core-> rst_i = 0;
	}

	virtual void tick(void) 
    {
		// Increment our own internal time reference
		tickcount ++;
		tickcount_total++;

        // Make sure any combinatorial logic depending upon
		// inputs that may have changed before we called tick()
		// has settled before the rising edge of the clock.
		m_core -> clk_i = 0;
		m_core -> eval();

		// ---------- Toggle the clock ------------

		// Rising edge
		m_core -> clk_i = 1;
		m_core -> eval();

		// Falling edge
		m_core -> clk_i = 0;
		m_core -> eval();
	}

	virtual bool  done(void)
	{
		return (Verilated::gotFinish()); 
	}
};


class Memory
{
	public:
	uint8_t * mem;
	uint32_t size;

	Memory(uint32_t max_addr, std::string filename)
	{
		size = max_addr;

		// Allocate memory
		if(!(mem = new uint8_t[max_addr])) 
		{
			std::cerr << "!Error: out of memory; memory allocation failed" <<std::endl;
			exit(1);
		}

		// Initialize with ff
		std::vector<char> fcontents = fReadBin(filename);
		for(unsigned int i = 0; i<fcontents.size(); i++)
		{
			mem[i] = (uint8_t)fcontents[i];
		}
	}

	~Memory()
	{
		delete [] mem;
		size = 0;
	}

	bool isValidAddress(uint32_t addr)
	{
		return (addr < size-4);
	}

	uint32_t fetchWord(uint32_t addr)
	{
		uint32_t data;
		if(!isValidAddress(addr))
			std::cerr << "!Error : Address out of bounds " << addr << "\n";
		else
		{
			uint32_t byte0 = (uint32_t)mem[addr];
			uint32_t byte1 = (uint32_t)mem[addr+1];
			uint32_t byte2 = (uint32_t)mem[addr+2];
			uint32_t byte3 = (uint32_t)mem[addr+3];

			return (byte3<<24 & 0xff000000) | (byte2<<16 & 0x00ff0000) |(byte1<<8 & 0x0000ff00) | (byte0 & 0x000000ff);
		}
		return 0;
	}
};


class Backend
{
	public:
	TESTBENCH<VAtomRVSoC> *tb;
	Memory * imem;

	enum CpuState{FETCH, EXECUTE};
	CpuState cpu_state;

	unsigned int pc;
	unsigned int rf[32];

	unsigned int ir;
	

	Backend(std::string imem_init_file)
	{
		tb = new TESTBENCH<VAtomRVSoC>();
		imem = new Memory(2000, imem_init_file);
		refreshData();
	}

	~Backend()
	{
		delete tb;
		delete imem;
	}

	void reset()
	{
		tb->reset();
	}

	void refreshData()
	{
		cpu_state = (CpuState)tb->m_core->AtomRVSoC->atom->ProcessorState;
		pc = tb->m_core->AtomRVSoC->atom->ProgramCounter;
		ir = tb->m_core->AtomRVSoC->atom->InstructionRegister;
		for(int i=0; i<32; i++)
			rf[i] = tb->m_core->AtomRVSoC->atom->rf->regs[i];
	}

	void displayData()
	{
		if(cpu_state == FETCH)
		{
			std::cout << "\nF-< "<<tb->tickcount<<" >\n";
			return;
		}
		std::cout << "E-< " << tb->tickcount <<" >------------------------------------------------------------\n";

		
		printf(" pc : 0x%08X   ir : 0x%08X \n\n", pc , ir); 

		int cols = 4;
		/*for(int i=0; i<32; i++)
		{
			printf("r%-2d: 0x%08X   ", i, rf[i]);
			if(i%cols == cols-1)
				printf("\n");
		}*/
		for(int i=0; i<32/cols; i++)
		{
			for(int j=0; j<cols; j++)
			{
				printf(" r%-2d: 0x%08X  ", i+4*j, rf[i]);
			}
			printf("\n");
		}

	}

	void tick()
	{
		tb->m_core->imem_data_i = imem->fetchWord(tb->m_core->imem_addr_o);
		tb->tick();
	}

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