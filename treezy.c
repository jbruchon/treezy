/* treezy (C) 2019-2020 Jody Bruchon <jody@jodybruchon.com>

   Licensed under The MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
//#include <sys/time.h>
#include <unistd.h>

#include "version.h"

#define ISFLAG(a,b) ((a & b) == b)
#define SETFLAG(a,b) (a |= b)
#define CLEARFLAG(a,b) (a &= (~b))

#define treezy_mode_t mode_t
#define treezy_ino_t ino_t

/* Behavior modification flags */
#define F_RECURSE		(1U << 0)

#define F_LOUD			(1U << 30)
#define F_DEBUG			(1U << 31)

#ifndef LOUD
 #define LOUD(a)
#endif

/* Per-file information */
struct file {
  struct _file *parent;
  struct _file *child;
  char *name;
  dev_t device;
  treezy_mode_t mode;
  off_t size;
  treezy_ino_t inode;
  time_t mtime;
  uint32_t flags;  /* Status flags */
#ifndef NO_HARDLINKS
  nlink_t nlink;
#endif
#ifndef NO_PERMS
  uid_t uid;
  gid_t gid;
#endif
};

struct filetree {
  struct file *file;
  struct filetree *left;
  struct filetree *right;
};

/* Suffix definitions (treat as case-insensitive) */
struct size_suffix {
  const char * const suffix;
  const int64_t multiplier;
};

const char *FILE_MODE_RO = "rb";
const char dir_sep[] = "/";

static const char *program_name;

static unsigned long errors = 0;

/* Larger chunk size makes large files process faster but uses more RAM */
#ifndef CHUNK_SIZE
 #define CHUNK_SIZE 32768
#endif

/* Maximum path buffer size to use; must be large enough for a path plus
 * any work that might be done to the array it's stored in. PATH_MAX is
 * not always true. Read this article on the false promises of PATH_MAX:
 * http://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
 */
#define PATHBUF_SIZE 1023
#define PATHBUF_SIZE_ALLOC 1024

/* Size suffixes - this gets exported */
static const struct size_suffix size_suffix[] = {
  /* Byte (someone may actually try to use this) */
  { "b", 1 },
  { "k", 1024 },
  { "kib", 1024 },
  { "m", 1048576 },
  { "mib", 1048576 },
  { "g", (uint64_t)1048576 * 1024 },
  { "gib", (uint64_t)1048576 * 1024 },
  { "t", (uint64_t)1048576 * 1048576 },
  { "tib", (uint64_t)1048576 * 1048576 },
  { "p", (uint64_t)1048576 * 1048576 * 1024},
  { "pib", (uint64_t)1048576 * 1048576 * 1024},
  { "e", (uint64_t)1048576 * 1048576 * 1048576},
  { "eib", (uint64_t)1048576 * 1048576 * 1048576},
  /* Decimal suffixes */
  { "kb", 1000 },
  { "mb", 1000000 },
  { "gb", 1000000000 },
  { "tb", 1000000000000 },
  { "pb", 1000000000000000 },
  { "eb", 1000000000000000000 },
  { NULL, 0 },
};

/* Progress indicator time */
//struct timeval time1, time2;

/* Basic type counts */
struct {
	int dirs;
	int files;
	int symlinks;
} count;


/***** End definitions, begin code *****/


/* Out of memory */
static void oom(const char * const restrict msg)
{
  fprintf(stderr, "\nout of memory: %s\n", msg);
  exit(EXIT_FAILURE);
}


/* Null pointer failure */
static void nullptr(const char * restrict func)
{
  static const char n[] = "(NULL)";
  if (func == NULL) func = n;
  fprintf(stderr, "\ninternal error: NULL pointer passed to %s\n", func);
  exit(EXIT_FAILURE);
}

int traverse_dir(const char *name, const int recurse)
{
	struct stat s;
	struct dirent *dir;
	DIR *cd;
	char *fullpath;
	char *tailpath = NULL;
	int result;

	LOUD(fprintf(stderr, "traverse_dir('%s', %d)\n", name, recurse););

	if (name == NULL) nullptr("traverse_dir");

	errno = 0;
	cd = opendir(name);
	if (!cd) goto error_opendir;

	fullpath = (char *)malloc(PATHBUF_SIZE_ALLOC * sizeof(char));
	if (!fullpath) oom("fullpath");

	*fullpath = '\0';
	strcpy(fullpath, name);
	strcat(fullpath, dir_sep);
	tailpath = fullpath + strlen(fullpath);

	while ((dir = readdir(cd)) != NULL) {
		int isdir = 0;

		if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;

		strcpy(tailpath, dir->d_name);

		printf("%s:ino=%lu:dtype=%d", fullpath, dir->d_ino, dir->d_type);
		result = lstat(fullpath, &s);
		if (result != 0) {
			fprintf(stderr, "  lstat failed for %s\n", fullpath);
			errors++;
		} else {
			switch (dir->d_type) {
			case DT_REG: printf(":type=f"); count.files++; break;
			case DT_DIR: printf(":type=d"); count.dirs++; isdir = 1; break;
			case DT_LNK: printf(":type=l"); count.symlinks++; break;
			case DT_UNKNOWN:
			default:
				if (S_ISREG(s.st_mode)) { printf(":type=f"); count.files++; }
				else if (S_ISDIR(s.st_mode)) { printf(":type=d"); isdir = 1; count.dirs++; }
				else if (S_ISLNK(s.st_mode)) { printf(":type=l"); count.symlinks++; }
				break;
			}
		}
		printf("\n");
		errno = 0;

		/* recurse */
		if (recurse && isdir) traverse_dir(fullpath, 1);
	}

	free(fullpath);
	closedir(cd);
	return 0;

error_opendir:
	fprintf(stderr, "error opening directory '%s'\n", name);
	errors++;
	return -1;
}


int main(int argc, char **argv)
{
	count.dirs = 0; count.files = 0; count.symlinks = 0;

	if (traverse_dir(".", 1) < 0) exit(EXIT_FAILURE);

	printf("\nfiles %d, dirs %d, symlinks %d, errors %lu\n", count.files, count.dirs, count.symlinks, errors);

	if (errors == 0) exit(EXIT_SUCCESS);
	else exit(EXIT_FAILURE);

}
