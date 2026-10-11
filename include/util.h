#ifndef MYLS_UTIL_H
#define MYLS_UTIL_H

#include "types.h"

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);
char *join_path(const char *dir, const char *name);
void report_error(const char *path);
void print_name(const char *name, const Options *opts);

#endif
