#include "util.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

void *xmalloc(size_t size)
{
    void *p = malloc(size ? size : 1);
    if (!p) {
        perror("myls: malloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size ? size : 1);
    if (!p) {
        perror("myls: realloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = xmalloc(n);
    memcpy(p, s, n);
    return p;
}

char *join_path(const char *dir, const char *name)
{
    size_t a = strlen(dir), b = strlen(name);
    bool slash = a && dir[a - 1] != '/';
    if (a > SIZE_MAX - b - 2) {
        fprintf(stderr, "myls: path too long\n");
        exit(EXIT_FAILURE);
    }
    char *p = xmalloc(a + b + 2);
    memcpy(p, dir, a);
    if (slash) p[a++] = '/';
    memcpy(p + a, name, b + 1);
    return p;
}

void report_error(const char *path)
{
    int saved = errno;
    fprintf(stderr, "myls: %s: %s\n", path, strerror(saved));
}

void print_name(const char *name, const Options *opts)
{
    if (!opts->quote) {
        fputs(name, stdout);
        return;
    }
    mbstate_t state = {0};
    size_t left = strlen(name);
    while (left) {
        /* Decode multibyte names so -q only replaces non-printable characters. */
        wchar_t wc;
        size_t n = mbrtowc(&wc, name, left, &state);
        if (n == (size_t)-1 || n == (size_t)-2) {
            putchar('?');
            n = 1;
            memset(&state, 0, sizeof(state));
        } else if (!n) {
            break;
        } else if (iswprint(wc)) {
            fwrite(name, 1, n, stdout);
        } else {
            putchar('?');
        }
        name += n;
        left -= n;
    }
}
