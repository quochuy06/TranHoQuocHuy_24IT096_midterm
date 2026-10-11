#ifndef MYLS_TYPES_H
#define MYLS_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>

typedef enum {
    SORT_NAME,
    SORT_TIME,
    SORT_SIZE
} SortKey;

typedef enum {
    TIME_MODIFIED,
    TIME_CHANGED,
    TIME_ACCESSED
} TimeKey;

typedef enum {
    UNITS_BLOCKS,
    UNITS_KILO,
    UNITS_HUMAN
} Units;

typedef struct {
    bool all;
    bool almost_all;
    bool directory;
    bool classify;
    bool unsorted;
    bool inode;
    bool long_format;
    bool numeric;
    bool quote;
    bool recursive;
    bool reverse;
    bool blocks;
    SortKey sort;
    TimeKey time_key;
    Units units;
    uintmax_t block_size;
} Options;

typedef struct {
    char *name;
    char *path;
    struct stat st;
} Entry;

typedef struct {
    Entry *items;
    size_t count;
    size_t capacity;
} Entries;

#endif
