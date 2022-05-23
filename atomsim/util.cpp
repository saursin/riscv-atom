#include "util.hpp"

#include <iostream>
#include <fstream>

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


std::vector<char> fReadBin(std::string memfile)
{        
    std::vector<char> fcontents;
    std::ifstream f (memfile, std::ios::out | std::ios::binary);
    
    if(!f)
    {
        throw "file access failed";
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
        throw "file reading failed!";
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
        throw "file access failed";
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
        throw "file writing failed";
    }
    for(unsigned int i=0; i<data.size(); i++)
    {
        File << data[i] <<"\n";
    }
    File.close();
}