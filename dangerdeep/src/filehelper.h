// file helper functions
// (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <string>
using namespace std;

// directory reading/writing encapsulated for compatibility

#ifdef WIN32
#include <windows.h>
typedef HANDLE directory;
#define PATHSEPARATOR "\\"
#else
#include <dirent.h>
typedef DIR* directory;
#define PATHSEPARATOR "/"
#endif

// file helper interface

directory open_dir(const string& filename);	// returns 0 if directory doesn't exist
string read_dir(directory d);	// returns empty string if directory is fully read.
void close_dir(directory d);
bool make_dir(const string& dirname);	// returns true on success
string get_current_directory(void);	// return absolute path
bool is_directory(const string& filename);	// test if file is directory

#endif
