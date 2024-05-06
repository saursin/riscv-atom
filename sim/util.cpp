#include "util.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "except.hpp"

// declared in main.cpp
extern bool NO_COLOR_OUTPUT;

static const std::map<ColorTag_t, char*> colormap = {
    {S_BOLD,     (char*)"\033[1m"},  {SN_BOLD,    (char*)"\033[22m"},    // set/unset bold mode.
    {S_DIM,      (char*)"\033[2m"},  {SN_DIM,     (char*)"\033[22m"},    // set/unset dim/faint mode.
    {S_ITALIC,   (char*)"\033[3m"},  {NS_ITALIC,  (char*)"\033[23m"},    // set/unset italic mode.
    {S_ULINE,    (char*)"\033[4m"},  {NS_ULINE,   (char*)"\033[24m"},    // set/unset underline mode.
    {S_BLINK,    (char*)"\033[5m"},  {NS_BLINK,   (char*)"\033[25m"},    // set/unset blinking mode
    {FG_BLACK,	 (char*)"\033[30m"}, {FG_BLACK,   (char*)"\033[40m"},
    {FG_RED,	 (char*)"\033[31m"}, {FG_RED,     (char*)"\033[41m"},
    {FG_GREEN,	 (char*)"\033[32m"}, {FG_GREEN,   (char*)"\033[42m"},
    {FG_YELLOW,	 (char*)"\033[33m"}, {FG_YELLOW,  (char*)"\033[43m"},
    {FG_BLUE,	 (char*)"\033[34m"}, {FG_BLUE,    (char*)"\033[44m"},
    {FG_MAGENTA, (char*)"\033[35m"}, {FG_MAGENTA, (char*)"\033[45m"},
    {FG_CYAN,	 (char*)"\033[36m"}, {FG_CYAN,    (char*)"\033[46m"},
    {FG_WHITE,	 (char*)"\033[37m"}, {FG_WHITE,   (char*)"\033[47m"},
    {FG_DEFAULT, (char*)"\033[39m"}, {FG_DEFAULT, (char*)"\033[49m"},
    {FG_RESET,	 (char*)"\033[0m"},  {FG_RESET,   (char*)"\033[0m"},
    {NULLSTR,    (char*)""}
};

char* ansicode(ColorTag_t tag) {
    if(NO_COLOR_OUTPUT) return colormap.at(NULLSTR);
    auto it = colormap.find(tag);
    return (char*)it->second;
}

void throwError(std::string er_code, std::string message, bool Exit)
{
    std::cerr << ansicode(FG_RED) <<"! ERROR "<< ansicode(FG_RESET) <<"["<< er_code <<"]: " << message << std::endl;
    
    if(Exit) 
        exit(EXIT_FAILURE);
}


void throwWarning(std::string wr_code, std::string message)
{
    std::cerr << ansicode(FG_YELLOW) <<"! WARNING " << ansicode(FG_RESET) << "[" << wr_code <<"]: " << message << std::endl;
}


void throwSuccessMessage(std::string message, bool Exit)
{
    if (NO_COLOR_OUTPUT)
        std::cout << "SUCCESS : " << message <<std::endl;
    else
        std::cout << ansicode(FG_GREEN) <<"SUCCESS " << ansicode(FG_RESET) << ": " << message <<std::endl;

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


std::string trimstr(std::string str, size_t len)
{
    if(str.length() > len){
        str.resize(len-3);
        str+=" ..";
    }
    else if (str.length()< len)
        str.append(len-str.length(), ' ');
    return str;
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


void getDisassembly(std::map<uint32_t, DisassembledLine> *dis, std::string filename)
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
	
	std::string line;
    bool in_disassembly = false;
	while(std::getline(s, line))
	{
        line = strip(line);

        // Skip blank lines
        if(line.length() == 0)
            continue;

        // replace tabs with spaces
        const int spacesPerTab = 2;
        for(size_t pos = 0; (pos = line.find('\t', pos)) != std::string::npos; pos += spacesPerTab) {
            // Replace each tab with spaces
            line.replace(pos, 1, spacesPerTab, ' ');
        }

        if(!in_disassembly) {
            if(line.find("Disassembly of section") != std::string::npos)
                in_disassembly = true;
            continue;
        }

        if(line == "...")
            break;


        // Tokenize
        std::stringstream ss(line);
        std::string tmp;
        std::vector<std::string> tok;
        while (ss >> tmp)
            tok.push_back(tmp);

        // Dism if tok 0's last char == ':'
        if(tok[0].back() == ':')
        {
            // remove colon from 1st token
            tok[0] = tok[0].substr(0, tok[0].length()-1);
            uint32_t addr = std::stoul(tok[0], 0, 16);
            uint32_t instrn = std::stoul(tok[1], 0, 16);
            std::string dism = strip(line.substr(line.find(tok[2])));

            DisassembledLine dl = {
                .instr = instrn,
                .disassembly = dism
            };

            // insert if not already present
            if(dis->count(addr) == 0) {
                dis->insert({addr, dl});
            }
        }
        else {
            // Parse the rest
            // printf("Unk[%s]\n", line.c_str());
        }
    }
}