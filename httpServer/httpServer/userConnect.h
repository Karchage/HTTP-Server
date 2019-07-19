#include "httpServer.h"
class userConnect
{
public:
	SOCKET *sok;
	char * inf;
	static void ClientHandler(userConnect User);
};