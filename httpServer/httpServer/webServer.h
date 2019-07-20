#include "httpServer.h"
class webServer 
{
public:
	void checkInfoFromxml();

private:
	std::string ipInform;
	//uint16_t setSizeUser;
	uint16_t portInform;
	void webServerRun();
	void getPoll(SOCKET lsock);
	uint8_t dontblockSocket(SOCKET fd);
};