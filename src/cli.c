#include "cli.h"
#include "errors.h"
#include <p101_c/p101_ctype.h>
#include <p101_c/p101_stdio.h>
#include <p101_c/p101_string.h>
#include <p101_cli/p101_getopt.h>
#include <p101_cli/p101_stdlib.h>
#include <p101_cli/p101_unistd.h>
#include <p101_convert/integer.h>
#include <p101_text/p101_ctype.h>
#include <p101_text/p101_regex.h>
#include <p101_text/p101_stdlib.h>
#include <p101_text/p101_string.h>
#include <p101_text/p101_strings.h>
#include <p101_text/p101_unistd.h>
#include <p101_text/p101_wchar.h>
#include <p101_text/p101_wctype.h>
#include <p101_text/p101_wordexp.h>
#include <stdio.h>
#include <stdlib.h>

static void parse_arguments(const struct p101_env *env, struct p101_error *err, int argc, char *argv[], struct arguments *args);
static void check_arguments(const struct p101_env *env, struct p101_error *err, const struct arguments *args);
static void convert_arguments(const struct p101_env *env, struct p101_error *err, struct arguments *args);

enum
{
    MSG_LEN = 256
};

void process_arguments(const struct p101_env *env, struct p101_error *err, int argc, char *argv[], struct arguments *args)
{
    P101_TRACE(env);
    parse_arguments(env, err, argc, argv, args);

    if(args->show_help || p101_error_has_error(err))
    {
        goto done;
    }

    check_arguments(env, err, args);

    if(p101_error_has_error(err))
    {
        goto done;
    }

    convert_arguments(env, err, args);

done:
    return;
}

static void parse_arguments(const struct p101_env *env, struct p101_error *err, int argc, char *argv[], struct arguments *args)
{
    int opt;

    P101_TRACE(env);

    opterr = 0;

    while((opt = p101_getopt(env, argc, argv, ":hvVd:")) != -1 && p101_error_has_no_error(err))
    {
        switch(opt)
        {
            case 'h':
            {
                args->show_help = true;
                break;
            }
            case 'v':
            {
                args->verbose = true;
                break;
            }
            case 'V':
            {
                args->fsm_verbose = true;
                break;
            }
            case 'd':
            {
                if(args->delay_str != NULL)
                {
                    P101_ERROR_RAISE_USER(err, "Option '-d' specified more than once.", ERR_USAGE);
                }

                if(optarg == NULL || optarg[0] == '\0')
                {
                    P101_ERROR_RAISE_USER(err, "Option '-d' requires a non-empty value.", ERR_USAGE);
                }

                args->delay_str = optarg;
                break;
            }
            case ':':
            {
                char msg[MSG_LEN];

                p101_snprintf(env, err, msg, sizeof msg, "Option '-%c' requires an argument.", optopt ? optopt : '?');
                P101_ERROR_RAISE_USER(err, msg, ERR_USAGE);
                break;
            }
            case '?':
            {
                char msg[MSG_LEN];

                if(p101_isprint(env, optopt))
                {
                    p101_snprintf(env, err, msg, sizeof msg, "Unknown option '-%c'.", optopt);
                }
                else
                {
                    p101_snprintf(env, err, msg, sizeof msg, "Unknown option character 0x%02X.", (unsigned)(unsigned char)optopt);
                }

                P101_ERROR_RAISE_USER(err, msg, ERR_USAGE);
                break;
            }
            default:
            {
                char msg[MSG_LEN];

                p101_snprintf(env, err, msg, sizeof msg, "Internal error: unhandled option '-%c' returned by getopt.", p101_isprint(env, opt) ? opt : '?');
                P101_ERROR_RAISE_USER(err, msg, ERR_USAGE);
                break;
            }
        }
    }

    if(p101_error_has_no_error(err) && optind < argc)
    {
        char   msg[MSG_LEN];
        size_t off;

        off = 0;
        off += (size_t)p101_snprintf(env, err, msg + off, sizeof msg - off, "Unexpected argument%s:", (argc - optind) > 1 ? "s" : "");

        for(int i = optind; i < argc && off < sizeof msg; ++i)
        {
            size_t rem;

            rem = sizeof msg - off;

            if(rem <= 1U)
            {
                break;
            }

            msg[off++] = ' ';
            rem        = sizeof msg - off;

            if(rem > 0U)
            {
                size_t ncopy;

                ncopy = p101_strnlen(env, argv[i], rem - 1U);
                p101_memcpy(env, msg + off, argv[i], ncopy);
                off += ncopy;
                msg[off] = '\0';
            }
        }

        msg[sizeof msg - 1U] = '\0';
        P101_ERROR_RAISE_USER(err, msg, ERR_USAGE);
    }
}

static void check_arguments(const struct p101_env *env, struct p101_error *err, const struct arguments *args)
{
    P101_TRACE(env);

    if(args->delay_str == NULL || args->delay_str[0] == '\0')
    {
        P101_ERROR_RAISE_USER(err, "The delay is required.", ERR_USAGE);
    }
}

static void convert_arguments(const struct p101_env *env, struct p101_error *err, struct arguments *args)
{
    P101_TRACE(env);

    args->delay = p101_parse_unsigned_int(env, err, args->delay_str, 0);

    if(p101_error_has_error(err))
    {
        P101_ERROR_RAISE_USER(err, "delay must be a positive integer.", ERR_USAGE);
    }
}

void print_usage(const struct p101_env *env, struct p101_error *err, const char *program_name, int exit_code, const char *message)
{
#ifndef P101_SUPPRESS_USAGE_TEXT
    FILE *stream;
#endif

    P101_TRACE(env);

#ifndef P101_SUPPRESS_USAGE_TEXT
    stream = (exit_code == EXIT_SUCCESS) ? stdout : stderr;

    if(message)
    {
        p101_fprintf(env, err, stream, "%s\n\n", message);
    }

    p101_fprintf(env, err, stream, "Usage: %s [-h] [-v] [-V] -d <delay>\n", program_name);
    p101_fputs(env, err, "Options:\n", stream);
    p101_fputs(env, err, "  -h                Display this help message and exit\n", stream);
    p101_fputs(env, err, "  -v                Enable verbose tracing\n", stream);
    p101_fputs(env, err, "  -V                Enable FSM state-change notifiers\n", stream);
    p101_fputs(env, err, "  -d <delay>        delay in seconds (required)\n", stream);
#else
    (void)err;
    (void)exit_code;
    (void)program_name;
    (void)message;
#endif
}
