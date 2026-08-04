/*
 * libFuzzer harness for the template's OWN argument parser
 * (src/cli.c: parse_arguments()). This fuzzes the code YOU write, not a
 * library function.
 *
 * parse_arguments() is static, so we #include the whole cli.c: that makes it
 * visible AND compiles it WITH the fuzzer instrumentation, which is what makes
 * this coverage-guided (watch "cov:" climb) instead of a blind black-box run.
 * The parser reports help and errors as data, so every input returns normally
 * to the harness.
 */
#include <p101_c/p101_stdlib.h>
#include <p101_c/p101_string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* The code under test. */
#include "../src/cli.c"

#define FUZZ_MAX_ARGS 64

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    char              *buf;
    char              *argv[FUZZ_MAX_ARGS];
    int                argc;
    char              *p;
    struct p101_error *err;
    struct p101_env   *env;
    struct arguments   args;

    err = p101_error_create(false);
    env = p101_env_create(err, NULL);

    /* getopt/argv need a writable, NUL-terminated C string. */
    buf = (char *)p101_malloc(env, err, size + 1);
    if(buf == NULL)
    {
        goto done;
    }
    p101_memcpy(env, buf, data, size);
    buf[size] = '\0';

    /* Carve the input into an argv, splitting on whitespace. argv[0] is a fixed
     * program name; the fuzzer controls every token after it. */
    argv[0] = (char *)"prog";
    argc    = 1;
    p       = buf;
    while(argc < FUZZ_MAX_ARGS - 1)
    {
        while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\v' || *p == '\f')
        {
            p++;
        }
        if(*p == '\0')
        {
            break;
        }
        argv[argc++] = p;
        while(*p != '\0' && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && *p != '\v' && *p != '\f')
        {
            p++;
        }
        if(*p != '\0')
        {
            *p++ = '\0';
        }
    }
    argv[argc] = NULL;

    /* getopt keeps a global cursor across calls; reset it before every parse. */
#ifdef __GLIBC__
    optind = 0; /* glibc: 0 forces a full re-init */
#else
    {
        extern int optreset; /* BSD / macOS / FreeBSD getopt */
        optreset = 1;
        optind   = 1;
    }
#endif

    p101_memset(env, &args, 0, sizeof(args));

    parse_arguments(env, err, argc, argv, &args);

done:
    p101_free(env, buf);
    p101_env_destroy(env);
    p101_error_destroy(err);
    return 0;
}
