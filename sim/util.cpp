#include "util.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "except.hpp"

// declared in main.cpp
extern bool NO_COLOR_OUTPUT;

void throwError(std::string er_code, std::string message, bool Exit)
{
    if (NO_COLOR_OUTPUT)
        std::cerr << "! ERROR ["<< er_code <<"]: " << message << std::endl;
    else
        std::cerr << COLOR_RED <<"! ERROR "<< COLOR_RESET <<"["<< er_code <<"]: " << message << std::endl;
    
    if(Exit) 
        exit(EXIT_FAILURE);
}


void throwWarning(std::string wr_code, std::string message)
{
    if (NO_COLOR_OUTPUT)
        std::cerr << "! WARNING [" << wr_code <<"]: " << message << std::endl;
    else
        std::cerr << COLOR_YELLOW <<"! WARNING " << COLOR_RESET << "[" << wr_code <<"]: " << message << std::endl;
}


void throwSuccessMessage(std::string message, bool Exit)
{
    if (NO_COLOR_OUTPUT)
        std::cout << "SUCCESS : " << message <<std::endl;
    else
        std::cout << COLOR_GREEN <<"SUCCESS " << COLOR_RESET << ": " << message <<std::endl;

    if(Exit)
        exit(EXIT_SUCCESS);
}


std::string lStrip(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}


std::string rStrip(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}


std::string strip(const std::string& s)
{
    return rStrip(lStrip(s));
}


size_t tokenize(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}

std::string resolve_envvar_in_path(std::string path) {
    std::string resolvedpath = path;
    size_t startpos = resolvedpath.find("${");

    while(startpos != std::string::npos) {
        size_t endpos = resolvedpath.find("}", startpos);
        if (endpos == std::string::npos) {
            throw Atomsim_exception("Malformed Path, can't resolve environment vars");
        }

        std::string var = resolvedpath.substr(startpos+2, endpos-startpos-2);
        const char * varval = getenv(var.c_str());

        if (varval == nullptr) {
            throw Atomsim_exception("Can't resolve envvar: " + var);
        }

        resolvedpath.replace(startpos, endpos-startpos+1, varval);
        startpos = resolvedpath.find("${", startpos + 1);
    }
    return resolvedpath;
}

std::vector<char> fReadBin(std::string filepath)
{        
    std::vector<char> fcontents;
    std::ifstream f (filepath, std::ios::out | std::ios::binary);
    
    if(!f)
    {
        throw Atomsim_exception("file access failed: "+filepath);
    }
    try
    {
        while(!f.eof())
        {    
            char byte;
            f.read((char *) &byte, 1);
            fcontents.push_back(byte);
        }
    }
    catch(...)
    {
        throw Atomsim_exception("file reading failed: "+filepath);
    }
    f.close();
    return fcontents;
}


std::vector<std::string> fRead (std::string filepath)
{
    // returns a vector of strings
    std::vector<std::string> text;

    // input file stream
    std::ifstream fin(filepath.c_str());
    if(!fin){
        throw Atomsim_exception("file access failed: "+filepath);
    }

    // reading file line by line and appending into the vector of strings
    std::string raw_line;
    while(getline(fin,raw_line))
    {
        text.push_back(raw_line);
    }

    // close file
    fin.close();
    return text;
}


void fWrite (std::vector<std::string> data, std::string filepath)
{
    std::ofstream File(filepath);
    if(!File)
    {
        throw Atomsim_exception("file writing failed: " + filepath);
    }
    for(unsigned int i=0; i<data.size(); i++)
    {
        File << data[i] <<"\n";
    }
    File.close();
}


std::string GetStdoutFromCommand(std::string cmd, bool get_output = true) {

  std::string data;
  FILE * stream;
  const int max_buffer = 256;
  char buffer[max_buffer];
  cmd.append(" 2>&1");

  stream = popen(cmd.c_str(), "r");

  if (get_output && stream) {
    while (!feof(stream))
      if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
    pclose(stream);
  }
  return data;
}


std::map<uint32_t, DisassembledLine> getDisassembly(std::string filename)
{
	std::string command = "";
	#ifdef RV32_COMPILER
		command +="riscv32-unknown-elf-objdump -d ";
	#else
		command += "riscv64-unknown-elf-objdump -d ";
	#endif
	command+=filename;
	
	// Get command output
	std::string output = GetStdoutFromCommand(command);
	
	std::stringstream s(output);

	// Parse command output
	std::map<uint32_t, DisassembledLine> dis;
	
	std::string line;
	while(std::getline(s, line))
	{        
        if(strip(line).length() == 0)
            continue;

        if(!(line.back() == ':' || line[0] != ' '))
        {
    
            line = strip(line);
            unsigned int colonAt = line.find(':');

            uint32_t addr = std::stoi(strip(line.substr(0, colonAt)), 0, 16);
            DisassembledLine d;
            d.instr = (uint32_t)std::stol(strip(line.substr(colonAt+1, colonAt+15)), 0, 16);
            d.disassembly = strip(line.substr(colonAt+16));

            dis.insert({addr, d});
        }
	}
	return dis;
}