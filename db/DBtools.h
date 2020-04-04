// author: sumingrui

#ifndef DBTOOL_H_
#define DBTOOL_H_

#include <vector>
#include <string>

using std::string;

struct hyperspectral_data_info
{
	string filename;
	uint16_t resolution_w;
	uint16_t resolution_h;
	uint16_t bands;
	string date;
	uint16_t datatype;
	float longitude;
	float latitude;
	float altitude;
	float height;
};

typedef hyperspectral_data_info HINFO;

int GetFilesInDir(string dbPath, std::vector<string> &ofiles);
int ReadConfig(string dbPath, string filename, HINFO &hinfo);

#endif // DBTOOL_H_