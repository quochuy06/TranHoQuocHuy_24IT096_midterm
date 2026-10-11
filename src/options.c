#include "options.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Parse BLOCKSIZE values used by -s totals when -h and -k are not selected. */
static uintmax_t environment_block_size(void)
{
    const char *s = getenv("BLOCKSIZE");
    if (!s || !*s)
        return 512;
    char *end;
    errno = 0;
    unsigned long long n = strtoull(s, &end, 10);
    unsigned power = 0;
    if (*end) {
        switch (*end) {
        case 'k':
        case 'K':
            power = 1;
            break;
        case 'm':
        case 'M':
            power = 2;
            break;
        case 'g':
        case 'G':
            power = 3;
            break;
        case 't':
        case 'T':
            power = 4;
            break;
        default:
            return 512;
        }
        if (end[1])
            return 512;
    }
    if (end == s || errno || !n || s[0] == '-')
        return 512;
    while (power--) {
        if (n > UINTMAX_MAX / 1024)
            return 512;
        n *= 1024;
    }
    return (uintmax_t)n;
}

int parse_options(int argc, char **argv, Options *o)
{
    memset(o, 0, sizeof(*o));
    o->almost_all = geteuid() == 0;
    o->quote = isatty(STDOUT_FILENO) != 0;
    o->block_size = environment_block_size();
    opterr = 0;
    int ch;
    /* Leading '+' stops option parsing at the first operand, preserving order. */
    while ((ch = getopt(argc, argv, "+AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'A':
            o->almost_all = true;
            break;
        case 'a':
            o->all = true;
            break;
        case 'c':
            o->time_key = TIME_CHANGED;
            break;
        case 'd':
            o->directory = true;
            o->recursive = false;
            break;
        case 'F':
            o->classify = true;
            break;
        case 'f':
            o->unsorted = true;
            break;
        case 'h':
            o->units = UNITS_HUMAN;
            break;
        case 'i':
            o->inode = true;
            break;
        case 'k':
            o->units = UNITS_KILO;
            break;
        case 'l':
            o->long_format = true;
            o->numeric = false;
            break;
        case 'n':
            o->long_format = true;
            o->numeric = true;
            break;
        case 'q':
            o->quote = true;
            break;
        case 'R':
            o->recursive = true;
            o->directory = false;
            break;
        case 'r':
            o->reverse = true;
            break;
        case 'S':
            o->sort = SORT_SIZE;
            break;
        case 's':
            o->blocks = true;
            break;
        case 't':
            o->sort = SORT_TIME;
            break;
        case 'u':
            o->time_key = TIME_ACCESSED;
            break;
        case 'w':
            o->quote = false;
            break;
        default:
            fprintf(stderr, "myls: invalid option -- '%c'\n"
                    "usage: myls [-AacdFfhiklnqRrSstuw] [file ...]\n", optopt);
            return -1;
        }
    }
    return optind;
}
