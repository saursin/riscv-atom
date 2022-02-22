#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

#include "svdpi.h"
#include "verilated_vpi.h"
#include "../../build/vobj_dir/VHydrogenSoC__Dpi.h"

extern "C" {
    extern void dpi_logger(const char* format);
    extern void dpi_logger_start();
    extern void dpi_logger_stop();
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
            logfile << s << std::flush;
    }
} dpi_logger_instance;



void dpi_logger(const char* format)
{
    if (!dpi_logger_instance.enabled)
        return;
    dpi_logger_instance.log_dump(format);
}

void dpi_logger_start()
{
    dpi_logger_instance.enable();
}

void dpi_logger_stop()
{
    dpi_logger_instance.disable();
}
