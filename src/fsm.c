/*
 * Copyright 2021-2026 D'Arcy Smith.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fsm.h"
#include "errors.h"
#include <p101_c/p101_stdio.h>
#include <p101_process/p101_unistd.h>
#include <p101_transition/transition.h>
#include <stdlib.h>

enum program_state
{
    PROGRAM_STATE_START = 0,
    PROGRAM_STATE_A,
    PROGRAM_STATE_B,
    PROGRAM_STATE_C,
    PROGRAM_STATE_DONE,
    PROGRAM_STATE_COUNT
};

enum
{
    PROGRAM_EVENT_ADVANCE = 1
};

static const struct p101_transition_rule program_rules[] = {
    {PROGRAM_STATE_START, PROGRAM_EVENT_ADVANCE, PROGRAM_STATE_A,    0U},
    {PROGRAM_STATE_A,     PROGRAM_EVENT_ADVANCE, PROGRAM_STATE_B,    0U},
    {PROGRAM_STATE_B,     PROGRAM_EVENT_ADVANCE, PROGRAM_STATE_C,    0U},
    {PROGRAM_STATE_C,     PROGRAM_EVENT_ADVANCE, PROGRAM_STATE_DONE, 0U},
};

static const char *program_state_name(enum program_state state);
static int         run_program_state(const struct p101_env *env, struct p101_error *err, const struct arguments *args, enum program_state state);

int run_fsm(const struct p101_env *env, struct p101_error *err, const struct arguments *args)
{
    int                           p101_single_result_;
    enum program_state            current_state;
    enum program_state            next_state;
    struct p101_transition_result transition;
    p101_transition_status        transition_status;
    p101_transition_id            transition_state;
    const char                   *current_name;
    const char                   *next_name;
    bool                          has_error;
    int                           state_status;
    int                           written;

    P101_TRACE(env);
    p101_single_result_ = EXIT_FAILURE;
    current_state       = PROGRAM_STATE_START;
    while(current_state != PROGRAM_STATE_DONE)
    {
        transition_state  = (p101_transition_id)current_state;
        transition_status = p101_transition_rules_find(program_rules, sizeof(program_rules) / sizeof(program_rules[0]), transition_state, PROGRAM_EVENT_ADVANCE, &transition);
        if(transition_status != P101_TRANSITION_OK)
        {
            P101_ERROR_RAISE_USER(err, "Program transition is not defined", ERR_FSM);
            goto p101_single_exit_;
        }

        next_state = (enum program_state)transition.rule->next_state;
        if(args->fsm_verbose)
        {
            current_name = program_state_name(current_state);
            next_name    = program_state_name(next_state);
            written      = p101_printf(env, err, "fsm will change from %s to %s\n", current_name, next_name);
            if(written < 0)
            {
                goto p101_single_exit_;
            }
        }

        state_status = run_program_state(env, err, args, next_state);
        if(state_status != EXIT_SUCCESS)
        {
            goto p101_single_exit_;
        }
        has_error = p101_error_has_error(err);
        if(has_error)
        {
            goto p101_single_exit_;
        }

        if(args->fsm_verbose)
        {
            current_name = program_state_name(current_state);
            next_name    = program_state_name(next_state);
            written      = p101_printf(env, err, "fsm did change from %s to %s\n", current_name, next_name);
            if(written < 0)
            {
                goto p101_single_exit_;
            }
        }
        current_state = next_state;
    }
    p101_single_result_ = EXIT_SUCCESS;

p101_single_exit_:
    P101_TRACE_EXIT(env);
    return p101_single_result_;
}

static const char *program_state_name(enum program_state state)
{
    const char *p101_single_result_;

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif
    switch(state)
    {
        case PROGRAM_STATE_START:
            p101_single_result_ = "start";
            break;
        case PROGRAM_STATE_A:
            p101_single_result_ = "a";
            break;
        case PROGRAM_STATE_B:
            p101_single_result_ = "b";
            break;
        case PROGRAM_STATE_C:
            p101_single_result_ = "c";
            break;
        case PROGRAM_STATE_DONE:
            p101_single_result_ = "done";
            break;
        default:
        case PROGRAM_STATE_COUNT:
            p101_single_result_ = "invalid";
            break;
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif

    return p101_single_result_;
}

static int run_program_state(const struct p101_env *env, struct p101_error *err, const struct arguments *args, enum program_state state)
{
    int          p101_single_result_;
    const char  *name;
    unsigned int remaining;
    int          written;

    p101_single_result_ = EXIT_SUCCESS;
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif
    switch(state)
    {
        case PROGRAM_STATE_A:
        case PROGRAM_STATE_B:
        case PROGRAM_STATE_C:
            name    = program_state_name(state);
            written = p101_printf(env, err, "%s called\n", name);
            if(written < 0)
            {
                p101_single_result_ = EXIT_FAILURE;
                break;
            }
            remaining = p101_sleep(env, args->delay);
            (void)remaining;
            break;
        case PROGRAM_STATE_START:
        case PROGRAM_STATE_DONE:
        case PROGRAM_STATE_COUNT:
            break;
        default:
            p101_single_result_ = EXIT_FAILURE;
            break;
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif

    return p101_single_result_;
}
