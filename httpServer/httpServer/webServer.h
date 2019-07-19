#include "httpServer.h"
class webServer
{
public:
	std::string ipInform;
	uint16_t portInform;
	void webServerRun();

private:
	void getPoll(SOCKET lsock);
	uint8_t dontblockSocket(SOCKET fd);
};