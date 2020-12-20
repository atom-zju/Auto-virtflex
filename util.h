#ifndef UTIL_H
#define UTIL_H

extern "C"
{
#include <xenstore.h>
}

int write_to_xenstore_path(const char* path, const char* value, int len);

int read_from_xenstore_path(const char* path, char* buf, int len);

#endif
