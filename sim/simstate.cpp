#include "simstate.hpp"
#include "util.hpp"

void Simstate::dump_simstate(std::string filename)
{
    std::vector<std::string> fcontents;
    char line[50];
    sprintf(line, "pc 0x%08x", state_.pc_e);
    fcontents.push_back(std::string(line));
    sprintf(line, "ir 0x%08x", state_.ins_e);
    fcontents.push_back(std::string(line));
    for (int i=0; i<32; i++) {
        sprintf(line, "x%d 0x%08x", i, state_.rf[i]);
        fcontents.push_back(std::string(line));
    }
    fWrite(fcontents, filename);
}
