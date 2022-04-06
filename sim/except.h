#pragma once

#include <exception>
#include <string>

class Atomsim_exception : public std::exception
{
public:
    Atomsim_exception(const std::string msg)
    : msg_(msg)
    {}

    char * what () 
    {
        return (char *)msg_.c_str();
    }

private:
    std::string msg_;
};
