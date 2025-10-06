#ifndef TEMPLATE_C_PROGRAM_ARGUMENTS_H
#define TEMPLATE_C_PROGRAM_ARGUMENTS_H

#include <stdbool.h>

struct arguments
{
    const char  *delay_str;
    unsigned int delay;
    bool         verbose;
    bool         fsm_verbose;
};

#endif    // TEMPLATE_C_PROGRAM_ARGUMENTS_H
