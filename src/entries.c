#include "entries.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void entries_add(Entries *list, const char *name, const char *path,
                 const struct stat *st)
{
    if (list->count == list->capacity) {
        size_t cap = list->capacity ? list->capacity * 2 : 32;
        if (cap < list->capacity || cap > SIZE_MAX / sizeof(Entry)) {
            fprintf(stderr, "myls: too many entries\n");
            exit(EXIT_FAILURE);
        }
        list->items = xrealloc(list->items, cap * sizeof(Entry));
        list->capacity = cap;
    }
    Entry *e = &list->items[list->count++];
    e->name = xstrdup(name);
    e->path = xstrdup(path);
    e->st = *st;
}

void entries_free(Entries *list)
{
    for (size_t i = 0; i < list->count; ++i) {
        free(list->items[i].name);
        free(list->items[i].path);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

struct timespec entry_time(const Entry *e, const Options *o)
{
    if (o->time_key == TIME_CHANGED)
        return e->st.st_ctim;
    if (o->time_key == TIME_ACCESSED)
        return e->st.st_atim;
    return e->st.st_mtim;
}

uintmax_t entry_blocks(const Entry *e)
{
    return e->st.st_blocks > 0 ? (uintmax_t)e->st.st_blocks : 0;
}

static int compare(const Entry *a, const Entry *b, const Options *o)
{
    int result = 0;
    if (o->sort == SORT_SIZE) {
        result = (a->st.st_size < b->st.st_size) -
                 (a->st.st_size > b->st.st_size);
    } else if (o->sort == SORT_TIME) {
        struct timespec ta = entry_time(a, o), tb = entry_time(b, o);
        result = (ta.tv_sec < tb.tv_sec) - (ta.tv_sec > tb.tv_sec);
        if (!result)
            result = (ta.tv_nsec < tb.tv_nsec) - (ta.tv_nsec > tb.tv_nsec);
    }
    if (!result) {
        int name = strcmp(a->name, b->name);
        result = (name > 0) - (name < 0);
    }
    return o->reverse ? -result : result;
}

/* Stable merge sort keeps equal entries in input order and avoids qsort globals. */
static void merge_sort(Entry *a, Entry *tmp, size_t lo, size_t hi,
                       const Options *o)
{
    if (hi - lo < 2)
        return;
    size_t mid = lo + (hi - lo) / 2;
    merge_sort(a, tmp, lo, mid, o);
    merge_sort(a, tmp, mid, hi, o);
    size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi)
        tmp[k++] = compare(&a[i], &a[j], o) <= 0 ? a[i++] : a[j++];
    while (i < mid)
        tmp[k++] = a[i++];
    while (j < hi)
        tmp[k++] = a[j++];
    memcpy(a + lo, tmp + lo, (hi - lo) * sizeof(*a));
}

void entries_sort(Entries *list, const Options *o)
{
    if (o->unsorted || list->count < 2)
        return;
    Entry *tmp = xmalloc(list->count * sizeof(*tmp));
    merge_sort(list->items, tmp, 0, list->count, o);
    free(tmp);
}
