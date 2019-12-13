/* treezy main program header
 * See treezy.c for license information */

#ifndef TREEZY_H
#define TREEZY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Detect Windows and modify as needed */
#if defined _WIN32 || defined __CYGWIN__
 #ifndef ON_WINDOWS
  #define ON_WINDOWS 1
 #endif
 #define NO_SYMLINKS 1
 #define NO_PERMS 1
 #define NO_DTYPE 1
 #define NO_SIGACTION 1
 #define nlink_t uint32_t
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #include <windows.h>
 #include <io.h>
 #include "win_stat.h"
#endif /* Win32 */

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include "version.h"

/* Some types are different on Windows */
#ifdef ON_WINDOWS
 typedef uint64_t treezy_ino_t;
 typedef uint32_t treezy_mode_t;
 extern const char dir_sep[];
 #ifdef UNICODE
  extern const wchar_t *FILE_MODE_RO;
 #else
  extern const char *FILE_MODE_RO;
 #endif /* UNICODE */

#else /* Not Windows */
 #include <sys/stat.h>
 typedef ino_t treezy_ino_t;
 typedef mode_t treezy_mode_t;
 extern const char *FILE_MODE_RO;
 extern const char dir_sep[];
 #ifdef UNICODE
  #error Do not define UNICODE on non-Windows platforms.
  #undef UNICODE
 #endif
#endif /* _WIN32 || __CYGWIN__ */

/* Windows + Unicode compilation */
#ifdef UNICODE
 #define WPATH_MAX 8192
 #define PATHBUF_SIZE WPATH_MAX-4
 #define PATHBUF_SIZE_ALLOC WPATH_MAX
  typedef wchar_t wpath_t[WPATH_MAX];
  extern wpath_t wname, wname2, wstr;
  extern int out_mode;
  extern int err_mode;
 #define M2W(a,b) MultiByteToWideChar(CP_UTF8, 0, a, -1, (LPWSTR)b, WPATH_MAX)
 #define W2M(a,b) WideCharToMultiByte(CP_UTF8, 0, a, -1, (LPSTR)b, WPATH_MAX, NULL, NULL)
#endif /* UNICODE */

#ifndef NO_SYMLINKS
/* #include "jody_paths.h" */
#endif

#define ISFLAG(a,b) ((a & b) == b)
#define SETFLAG(a,b) (a |= b)
#define CLEARFLAG(a,b) (a &= (~b))

/* Low memory option overrides */
#ifdef LOW_MEMORY
 #ifndef NO_PERMS
  #define NO_PERMS 1
 #endif
#endif

/* Aggressive verbosity for deep debugging */
#ifdef LOUD_DEBUG
 #ifndef DEBUG
  #define DEBUG
 #endif
 #define LOUD(...) if (1) __VA_ARGS__
/* #define LOUD(...) if ISFLAG(flags, F_LOUD) __VA_ARGS__ */
#else
 #define LOUD(a)
#endif

/* Compile out debugging stat counters unless requested */
#ifdef DEBUG
 #define DBG(a) a
#else
 #define DBG(a)
#endif


/* Behavior modification flags */
extern uint_fast32_t flags;
#define F_RECURSE		(1U << 0)

#define F_LOUD			(1U << 30)
#define F_DEBUG			(1U << 31)

/* Maximum path buffer size to use; must be large enough for a path plus
 * any work that might be done to the array it's stored in. PATH_MAX is
 * not always true. Read this article on the false promises of PATH_MAX:
 * http://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
 */
#ifndef PATHBUF_SIZE
#define PATHBUF_SIZE 4094
#define PATHBUF_SIZE_ALLOC 4096
#endif

/* Per-file information */
typedef struct _file {
  struct _file *duplicates;
  struct _file *next;
  char *d_name;
  dev_t device;
  treezy_mode_t mode;
  off_t size;
  treezy_ino_t inode;
  time_t mtime;
  uint32_t flags;  /* Status flags */
#ifndef NO_HARDLINKS
 #ifndef ON_WINDOWS
  nlink_t nlink;
 #else
  uint32_t nlink;  /* link count on Windows is always a DWORD */
 #endif
#endif
#ifndef NO_PERMS
  uid_t uid;
  gid_t gid;
#endif
} file_t;

typedef struct _filetree {
  file_t *file;
  struct _filetree *left;
  struct _filetree *right;
} filetree_t;

/* Pick a stat() function */
#ifdef ON_WINDOWS
 #define STAT win_stat
#else
 #define STAT lstat
#endif

/* -X exclusion parameter stack */
struct exclude {
  struct exclude *next;
  unsigned int flags;
  int64_t size;
  char param[];
};

/* Suffix definitions (treat as case-insensitive) */
struct size_suffix {
  const char * const suffix;
  const int64_t multiplier;
};

extern const struct size_suffix size_suffix[];
extern char tempname[PATHBUF_SIZE * 2];

extern void oom(const char * const restrict msg);
extern void nullptr(const char * restrict func);

#ifdef __cplusplus
}
#endif

#endif /* TREEZY_H */

