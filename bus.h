// Author: sumingrui

#ifndef BUS_H_
#define BUS_H_

#include "db/hyperspectralDB.h"

namespace HSIC
{

struct AppConfig
{
	// database config
	char hostName[64];
	char account[20];
	char passwd[20];
	char db[20];
	uint32_t port;
	char dataPath[128];

	// algorithm config
	int speed_level; // 1: slow  2:middle  3:fast
};

class Bus
{
public:
	Bus();
	~Bus();
	bool ReadAppConfig();
	void MultiModule();
	void ObjSearch();

private:
	AppConfig config_;
	SpectralDataSQL *pHSIDB;
};

} // namespace HSIC

#endif // BUS_H_