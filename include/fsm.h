#ifndef TEMPLATE_C_PROGRAM_FSM_H
#define TEMPLATE_C_PROGRAM_FSM_H

#include "arguments.h"
#include <p101_env/env.h>

/* A tiny data-driven machine built directly on the libc-only transition core. */
int run_fsm(const struct p101_env *env, struct p101_error *err, const struct arguments *args);

#endif    // TEMPLATE_C_PROGRAM_FSM_H
