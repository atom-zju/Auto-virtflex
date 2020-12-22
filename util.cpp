#include "util.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>


int write_to_xenstore_path(struct xs_handle* xs, const string path, const string value){
	
        xs_transaction_t th;

        int er;

        th = xs_transaction_start(xs);
        er = xs_write(xs, th, path.c_str(), value.c_str(), strlen(value.c_str()));
        xs_transaction_end(xs, th, false);

        if (er == 0){
                fprintf(stderr, "fail to write to path: %s\n", path.c_str());
                return -1;
        }
	
	return 0;

}

int read_from_xenstore_path(struct xs_handle* xs, const string path, string& s){
        
	xs_transaction_t th;

	char* buf;
	unsigned int len;
		
        th = xs_transaction_start(xs);
        buf = (char*)xs_read(xs, th, path.c_str(), &len);
        xs_transaction_end(xs, th, false);

        if (!buf){
                fprintf(stderr, "fail to read from path: %s\n", path.c_str());
                return -1;
        }
	s.clear();
	s.append(buf);	
	free(buf);
	return 0;

}

int list_xenstore_directory(struct xs_handle* xs, const string path, vector<string>& res){
	xs_transaction_t th;
	unsigned int num;
		
	char** list;
	th = xs_transaction_start(xs);
        list = xs_directory(xs, th, path.c_str(), &num);
        xs_transaction_end(xs, th, false);
	
	if(!list){
                fprintf(stderr, "fail to read xs directory: %s\n", path.c_str());
                return -1;
        }
	
	res.clear();	
	for(int i=0; i<num; i++){
		res.push_back(string(list[i]));
		//free(list[i]);
	}
	free(list);
	return 0;

}
