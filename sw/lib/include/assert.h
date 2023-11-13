#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <utils.h>


#define assert(x) \
if(!(x)) { \
    puts("[ERROR]: Assert Failed:" EXPAND_AND_STRINGIFY(__FILE__) EXPAND_AND_STRINGIFY(__LINE__));  \
    exit(1); \
}
