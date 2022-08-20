#pragma once
#include <stdint.h>

/**
 * @brief Struct to hold CPU state
 */
struct CPUState
{
    // Fetch-Stage
    unsigned int pc_f;
    unsigned int ins_f;

    // Execute Stage
    unsigned int pc_e;
    unsigned int ins_e;
    
    // Register File
    unsigned int rf[32];

    // Tick counts
    uint64_t tickcount_total;
    uint64_t tickcount;
};


/**
 * @brief Struct to hold CPU signal values
 */
struct CPUSignals
{
    bool jump_decision;
};


/**
 * @brief AtomSim Middle end
 */
class Simstate
{
public:
    /**
     * @brief CPU State
     */
    CPUState state_;

    /**
     * @brief CPU signal values
     */
    struct CPUSignals signals_;
};