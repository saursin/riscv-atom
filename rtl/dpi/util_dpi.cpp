#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

#include "svdpi.h"
#include "verilated_vpi.h"
#include "../../build/vobj_dir/VHydrogenSoC__Dpi.h"

extern "C" {
    extern void dpi_trace(const char* format);
    extern void dpi_trace_start();
    extern void dpi_trace_stop();
}


class DPI_Logger
{
    public:
    bool enabled = false;
    std::fstream  logfile;

    void enable()
    {
        logfile.open("run.log", std::ios::out | std::ios::trunc);
        enabled = true;
    }

    void disable()
    {
        logfile.close();
        enabled = false;
    }

    void log_dump(std::string s)
    {
        if(enabled)
            logfile << s;
    }
} dpi_logger;



void dpi_trace(const char* format)
{
    if (!dpi_logger.enabled)
        return;
    
    // char line[200];

    // // va_list va;
    // // va_start(va, format);
    // sprintf(line, format);//, va);
    // // va_end(va);

    dpi_logger.log_dump(format);
}

void dpi_trace_start()
{
    dpi_logger.enable();
}

void dpi_trace_stop()
{
    dpi_logger.disable();
}
