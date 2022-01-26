#ifndef TOTO_CHANGE_RECORD_H

class topo_change_record{
	public:
		bool ending;
		union{
			{	// if the record is starting record
				bool is_shrink;
				int from_node;
				int to_node;
			},
			{	// if the record is ending record
				int num_of_page_migrations;
			}
		}
};

#define TOPO_CHANGE_RECORD_H
