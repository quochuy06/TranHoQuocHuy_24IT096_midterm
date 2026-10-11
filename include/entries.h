#ifndef MYLS_ENTRIES_H
#define MYLS_ENTRIES_H

#include "types.h"

void entries_add(Entries *list, const char *name, const char *path,
                 const struct stat *st);
void entries_free(Entries *list);
void entries_sort(Entries *list, const Options *opts);
struct timespec entry_time(const Entry *entry, const Options *opts);
uintmax_t entry_blocks(const Entry *entry);

#endif
