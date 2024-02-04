#ifndef GIT_UTIL_H
#define GIT_UTIL_H
#include <git2.h>
#ifdef _WIN32
# include <io.h>
# include <Windows.h>
# define open _open
# define read _read
# define close _close
# define ssize_t int
# define sleep(a) Sleep(a * 1000)
#else
# include <unistd.h>
#endif

#ifndef PRIuZ
/* Define the printf format specifier to use for size_t output */
#if defined(_MSC_VER) || defined(__MINGW32__)
#	define PRIuZ "Iu"
#else
#	define PRIuZ "zu"
#endif
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#define strcasecmp strcmpi
#endif

#define UNUSED(x) (void)(x)

enum index_mode {
    INDEX_NONE,
    INDEX_ADD
};

struct index_options {
    int dry_run;
    int verbose;
    git_repository *repo;
    enum index_mode mode;
    int add_update;
};

struct args_info {
    int    argc;
    char **argv;
    int    pos;
    unsigned int opts_done : 1; /**< Did we see a -- separator */
};

#define ARGS_INFO_INIT { argc, argv, 0, 0 }

#endif // GIT_UTIL_H
