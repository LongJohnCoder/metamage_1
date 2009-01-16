/*	====
 *	at.h
 *	====
 */

#ifndef AT_H
#define AT_H

// POSIX
#include <dirent.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LAMP__

// time
int futimesat_k( int dirfd, const char* path, const struct timeval* access,
                                              const struct timeval* mod,
                                              const struct timeval* backup,
                                              const struct timeval* creat );

// unistd
ssize_t readlinkat_k( int dirfd, const char *path, char *buffer, size_t buffer_size );

#ifndef __linux__

// dirent
DIR *fdopendir( int fd );

// fcntl
int openat( int dirfd, const char* path, int flags, mode_t mode );

// stdio
int renameat( int olddirfd, const char* oldpath, int newdirfd, const char* newpath );

// sys/stat
int fstatat( int dirfd, const char* path, struct stat* sb, int flags );
int mkdirat( int dirfd, const char* path, mode_t mode );
int mkfifoat( int dirfd, const char* path, mode_t mode );
int mknodat( int dirfd, const char* path, mode_t mode, dev_t dev );

// time
int futimesat( int dirfd, const char* path, const struct timeval times[2] );

// unistd
int faccessat( int dirfd, const char* path, int mode, int flags );
int fchmodat( int dirfd, const char* path, mode_t mode, int flags );
int fchownat( int dirfd, const char* path, uid_t owner, gid_t group, int flags );
int fexecve( int fd, const char *const argv[], const char *const envp[] );
int linkat( const char* target_path, int newdirfd, const char* newpath, int flags );
ssize_t readlinkat( int dirfd, const char *path, char *buffer, size_t buffer_size );
int symlinkat( const char* target_path, int newdirfd, const char* newpath );
int unlinkat( int dirfd, const char* path, int flags );

#endif  // !__linux__

#endif  // !__LAMP__

#ifdef __cplusplus
}
#endif

#endif

