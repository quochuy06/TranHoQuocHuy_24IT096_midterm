#include "format.h"
#include "entries.h"
#include "util.h"
#include <errno.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/sysmacros.h>
#endif

static char file_type(mode_t m)
{
    if (S_ISDIR(m))
        return 'd';
    if (S_ISLNK(m))
        return 'l';
    if (S_ISCHR(m))
        return 'c';
    if (S_ISBLK(m))
        return 'b';
    if (S_ISSOCK(m))
        return 's';
    if (S_ISFIFO(m))
        return 'p';
#ifdef S_ISWHT
    if (S_ISWHT(m))
        return 'w';
#endif
    return '-';
}

static void mode_string(mode_t m, char out[11])
{
    const mode_t bits[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                           S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    const char *letters = "rwxrwxrwx";
    out[0] = file_type(m);
    for (int i = 0; i < 9; ++i)
        out[i + 1] = m & bits[i] ? letters[i] : '-';
    if (m & S_ISUID)
        out[3] = m & S_IXUSR ? 's' : 'S';
    if (m & S_ISGID)
        out[6] = m & S_IXGRP ? 's' : 'S';
#ifdef S_ISVTX
    if (m & S_ISVTX)
        out[9] = m & S_IXOTH ? 't' : 'T';
#elif defined(S_ISTXT)
    if (m & S_ISTXT)
        out[9] = m & S_IXOTH ? 't' : 'T';
#endif
    out[10] = '\0';
}

/* Human-readable sizes use 1024-based units, matching the project scope. */
static void human_size(uintmax_t bytes)
{
    const char *suffix = "BKMGTPE";
    long double value = (long double)bytes;
    unsigned i = 0;
    while (value >= 1024 && i < 6) {
        value /= 1024;
        ++i;
    }
    if (value >= 1023.5L && i < 6) {
        value /= 1024;
        ++i;
    }
    if (!i || value >= 10)
        printf("%.0Lf%c", value, suffix[i]);
    else
        printf("%.1Lf%c", value, suffix[i]);
}

static void block_count(uintmax_t blocks, const Options *o)
{
    uintmax_t bytes = blocks > UINTMAX_MAX / 512 ? UINTMAX_MAX : blocks * 512;
    if (o->units == UNITS_HUMAN) {
        human_size(bytes);
        return;
    }
    uintmax_t unit = o->units == UNITS_KILO ? 1024 : o->block_size;
    printf("%ju", bytes / unit + (bytes % unit != 0));
}

/* Owner/group names fall back to numeric IDs when lookup is unavailable. */
static void print_owner_group(const Entry *e, const Options *o)
{
    struct passwd *pw = o->numeric ? NULL : getpwuid(e->st.st_uid);
    if (pw)
        printf("%s ", pw->pw_name);
    else
        printf("%ju ", (uintmax_t)e->st.st_uid);
    struct group *gr = o->numeric ? NULL : getgrgid(e->st.st_gid);
    if (gr)
        printf("%s ", gr->gr_name);
    else
        printf("%ju ", (uintmax_t)e->st.st_gid);
}

static void print_date(const Entry *e, const Options *o)
{
    time_t seconds = entry_time(e, o).tv_sec;
    struct tm tm;
    char buf[64];
    if (localtime_r(&seconds, &tm) &&
        strftime(buf, sizeof(buf), "%b %e %H:%M", &tm))
        fputs(buf, stdout);
    else
        fputs("??? ?? ??:??", stdout);
}

static char classification(mode_t m)
{
    if (S_ISDIR(m))
        return '/';
    if (S_ISLNK(m))
        return '@';
    if (S_ISSOCK(m))
        return '=';
    if (S_ISFIFO(m))
        return '|';
#ifdef S_ISWHT
    if (S_ISWHT(m))
        return '%';
#endif
    if (S_ISREG(m) && (m & (S_IXUSR | S_IXGRP | S_IXOTH)))
        return '*';
    return '\0';
}

static char *link_target(const char *path)
{
    size_t cap = 128;
    for (;;) {
        char *buf = xmalloc(cap + 1);
        ssize_t n = readlink(path, buf, cap);
        if (n < 0) {
            free(buf);
            return NULL;
        }
        if ((size_t)n < cap) {
            buf[n] = '\0';
            return buf;
        }
        free(buf);
        if (cap > SIZE_MAX / 2 - 1) {
            errno = ENAMETOOLONG;
            return NULL;
        }
        cap *= 2;
    }
}

int format_failed = 0;

/* Print one entry using the active combination of inode, block, long and -F flags. */
void print_entry(const Entry *e, const Options *o)
{
    if (o->inode)
        printf("%ju ", (uintmax_t)e->st.st_ino);
    if (o->blocks) {
        block_count(entry_blocks(e), o);
        putchar(' ');
    }
    if (o->long_format) {
        char mode[11];
        mode_string(e->st.st_mode, mode);
        printf("%s %ju ", mode, (uintmax_t)e->st.st_nlink);
        print_owner_group(e, o);
        if (S_ISCHR(e->st.st_mode) || S_ISBLK(e->st.st_mode))
            printf("%ju, %ju", (uintmax_t)major(e->st.st_rdev),
                   (uintmax_t)minor(e->st.st_rdev));
        else if (o->units == UNITS_HUMAN)
            human_size(e->st.st_size < 0 ? 0 : (uintmax_t)e->st.st_size);
        else
            printf("%jd", (intmax_t)e->st.st_size);
        putchar(' ');
        print_date(e, o);
        putchar(' ');
    }
    print_name(e->name, o);
    if (o->classify) {
        char ch = classification(e->st.st_mode);
        if (ch) putchar(ch);
    }
    if (o->long_format && S_ISLNK(e->st.st_mode)) {
        char *target = link_target(e->path);
        if (target) {
            fputs(" -> ", stdout);
            print_name(target, o);
            free(target);
        } else {
            report_error(e->path);
            format_failed = 1;
        }
    }
    putchar('\n');
}

void print_total(const Entries *list, const Options *o)
{
    uintmax_t blocks = 0;
    for (size_t i = 0; i < list->count; ++i) {
        uintmax_t b = entry_blocks(&list->items[i]);
        if (b > UINTMAX_MAX - blocks) {
            blocks = UINTMAX_MAX;
            break;
        }
        blocks += b;
    }
    fputs("total ", stdout);
    block_count(blocks, o);
    putchar('\n');
}
