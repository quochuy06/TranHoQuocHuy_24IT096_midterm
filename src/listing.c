#include "listing.h"
#include "entries.h"
#include "format.h"
#include "util.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct Ancestor {
    dev_t device;
    ino_t inode;
    const struct Ancestor *parent;
} Ancestor;

typedef struct {
    const Options *opts;
    bool printed;
    int failed;
} Context;

static void print_header(Context *ctx, const char *path)
{
    if (ctx->printed)
        putchar('\n');
    print_name(path, ctx->opts);
    fputs(":\n", stdout);
    ctx->printed = true;
}

static bool visible(const char *name, const Options *o)
{
    if (name[0] != '.' || o->all)
        return true;
    return o->almost_all && strcmp(name, ".") && strcmp(name, "..");
}

/* List a directory, detect ancestor cycles, and recurse only into real directories. */
static void list_directory(Context *ctx, const char *path, bool header,
                           const Ancestor *parent, unsigned depth)
{
    struct stat dir_stat;
    if (stat(path, &dir_stat) < 0) {
        report_error(path);
        ctx->failed = 1;
        return;
    }
    for (const Ancestor *a = parent; a; a = a->parent) {
        if (a->device == dir_stat.st_dev && a->inode == dir_stat.st_ino) {
            fprintf(stderr, "myls: %s: recursive directory cycle\n", path);
            ctx->failed = 1;
            return;
        }
    }
    if (depth > 512) {
        fprintf(stderr, "myls: %s: recursion depth limit (512) exceeded\n", path);
        ctx->failed = 1;
        return;
    }
    Ancestor here = {dir_stat.st_dev, dir_stat.st_ino, parent};
    DIR *dir = opendir(path);
    if (!dir) {
        report_error(path);
        ctx->failed = 1;
        return;
    }
    if (header)
        print_header(ctx, path);
    Entries entries = {0};
    const Options *o = ctx->opts;
    for (;;) {
        /* errno must be reset immediately before readdir() to detect read errors. */
        errno = 0;
        struct dirent *de = readdir(dir);
        if (!de) {
            if (errno) {
                report_error(path);
                ctx->failed = 1;
            }
            break;
        }
        if (!visible(de->d_name, o))
            continue;
        char *full = join_path(path, de->d_name);
        struct stat st;
        if (lstat(full, &st) < 0) {
            report_error(full);
            ctx->failed = 1;
        } else {
            entries_add(&entries, de->d_name, full, &st);
        }
        free(full);
    }
    if (closedir(dir) < 0) {
        report_error(path);
        ctx->failed = 1;
    }
    entries_sort(&entries, o);
    if (o->long_format || (o->blocks && isatty(STDOUT_FILENO))) {
        print_total(&entries, o);
        ctx->printed = true;
    }
    for (size_t i = 0; i < entries.count; ++i) {
        print_entry(&entries.items[i], o);
        ctx->printed = true;
    }
    if (o->recursive) {
        for (size_t i = 0; i < entries.count; ++i) {
            Entry *e = &entries.items[i];
            if (S_ISDIR(e->st.st_mode) && strcmp(e->name, ".") &&
                strcmp(e->name, "..")) {
                list_directory(ctx, e->path, true, &here, depth + 1);
            }
        }
    }
    entries_free(&entries);
}

int list_operands(int argc, char **argv, const Options *o)
{
    Entries files = {0};
    Entries dirs = {0};
    Context ctx = {o, false, 0};
    char *current[] = {"."};
    if (!argc) {
        argc = 1;
        argv = current;
    }
    for (int i = 0; i < argc; ++i) {
        struct stat st;
        if (lstat(argv[i], &st) < 0) {
            report_error(argv[i]);
            ctx.failed = 1;
            continue;
        }
        /* Plain command-line symlinks to directories are traversed like NetBSD ls. */
        if (S_ISLNK(st.st_mode) && !o->directory && !o->long_format &&
            !o->classify) {
            struct stat target;
            if (stat(argv[i], &target) == 0 && S_ISDIR(target.st_mode))
                st = target;
        }
        Entries *destination =
            S_ISDIR(st.st_mode) && !o->directory ? &dirs : &files;
        entries_add(destination, argv[i], argv[i], &st);
    }
    entries_sort(&files, o);
    entries_sort(&dirs, o);
    for (size_t i = 0; i < files.count; ++i) {
        print_entry(&files.items[i], o);
        ctx.printed = true;
    }
    for (size_t i = 0; i < dirs.count; ++i)
        list_directory(&ctx, dirs.items[i].path, argc > 1 || o->recursive, NULL, 0);
    entries_free(&files);
    entries_free(&dirs);
    return ctx.failed;
}
