#ifndef MYLS_FORMAT_H
#define MYLS_FORMAT_H

#include "types.h"

void print_entry(const Entry *entry, const Options *opts);
void print_total(const Entries *list, const Options *opts);
extern int format_failed;

#endif
