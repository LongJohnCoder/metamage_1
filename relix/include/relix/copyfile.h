/*
	copyfile.h
	----------
	
	Joshua Juran
*/

#ifndef RELIX_COPYFILE_H
#define RELIX_COPYFILE_H

#ifdef __cplusplus
extern "C" {
#endif

int copyfile( const char* src, const char* dest );

int copyfileat( int olddirfd, const char* oldpath, int newdirfd, const char* newpath, unsigned flags );

#ifdef __cplusplus
}
#endif

#endif
