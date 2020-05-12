// Author: sumingrui

#ifndef BUS_H_
#define BUS_H_

#include "db/hyperspectralDB.h"

namespace HSIC
{

using std::string;

struct AppConfig
{
	// database config
	string hostName;
	string account;
	string passwd;
	string db;
	uint32_t port;
	string dataPath;

	// algorithm config
	int speed_level; // 1: slow  2:middle  3:fast
	int useNUC;
	double XavierComputeRatio;
};

class Bus
{
public:
	Bus();
	~Bus();
	bool ReadAppConfig();
	void ObjSearch();
	int AddHSIEntry(string filename);
	int DelHSIEntry(string filename);

private:
	AppConfig config_;
	SpectralDataSQL *pHSIDB;
};

} // namespace HSIC

#endif // BUS_H_