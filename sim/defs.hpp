/**
 * @brief Version information
 */
const char Info_version[] = "AtomSim v1.0";

/**
 * @brief Copyright message
 */
const char Info_copyright[] = 
"MIT License\n\n"

"Copyright (c) 2020 EUREG\n\n"

"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
"of this software and associated documentation files (the \"Software\"), to deal\n"
"in the Software without restriction, including without limitation the rights\n"
"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
"copies of the Software, and to permit persons to whom the Software is\n"
"furnished to do so, subject to the following conditions:\n\n"

"The above copyright notice and this permission notice shall be included in all\n"
"copies or substantial portions of the Software.\n\n"

"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
"SOFTWARE.\n";


/**
 * @brief Short help message
 */
const char Info_short_help_msg [] =
"<needs update> \n"
"Usage: atomsim  [-a] [-v] [-n] [-o <file>] [-h]\n" 
"                    [--help] [--version] [--time] file(s)\n";


/**
 * @brief long help message
 * 
 */
const char Info_long_help_msg [] =
"<needs update> \n"
"Usage: atomsim [options] file(s)\n\n"
"Options:\n"
"    -p              Preprocess only\n"
"    -a              Assemble only\n"
"    -v              Turn on verbose mode\n"
"    -n              Suppress warnings"
"    -o <file>       Place the output into <file>\n"
"    -h | --help     Display help information\n"
"    --version       Display Assembler version information\n"
"    --time          Time the execution of Assembler\n";




/**
 * @brief String stripping utility functions
 * 
 * @see https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/#:~:text=We%20can%20use%20combination%20of,functions%20to%20trim%20the%20string.
 */
const std::string WHITESPACE = " \n\r\t\f\v";

/**
 * @brief removes preceeding whitespaces in a string
 * 
 * @param s string
 * @return std::string 
 */
std::string lStrip(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

/**
 * @brief removes succeding whitespaces in a string
 * 
 * @param s string
 * @return std::string 
 */
std::string rStrip(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

/**
 * @brief removes preceding & succeding whitespaces in a string
 * 
 * @param s string
 * @return std::string 
 */
std::string strip(const std::string& s)
{
    return rStrip(lStrip(s));
}

// =============================== STRING TOKENIZING =====================================
/**
 * @brief splits a string accordint to delimiter 
 * 
 * @param txt input string
 * @param strs vector of strings parts
 * @param ch delimiter
 * @return size_t 
 */
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

/**
 * @brief reads a binary file
 * 
 * @param memfile filepath
 * @return std::vector<char> contents
 */
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