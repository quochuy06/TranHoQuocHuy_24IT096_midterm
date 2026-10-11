#include "options.h"
#include "listing.h"
#include "format.h"
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    setlocale(LC_CTYPE, "");
    signal(SIGPIPE, SIG_IGN);
    Options opts;
    int first = parse_options(argc, argv, &opts);
    if (first < 0)
        return EXIT_FAILURE;
    int failed = list_operands(argc - first, argv + first, &opts);
    if (fflush(stdout) == EOF || ferror(stdout)) {
        fprintf(stderr, "myls: failed to write standard output\n");
        failed = 1;
    }
    return failed || format_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
