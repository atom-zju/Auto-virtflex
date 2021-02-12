#ifndef UTIL_H
#define UTIL_H

#include<vector>
#include<string>
extern "C"
{
#include <xenstore.h>
#include <xenctrl.h>
#include <libxl.h>
}

using namespace std;


int write_to_xenstore_path(struct xs_handle* xs, const string path, const string value);

int read_from_xenstore_path(struct xs_handle* xs, const string path, string& s);

int list_xenstore_directory(struct xs_handle* xs, const string path, vector<string>& res);

#endif
