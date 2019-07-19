#include "httpServer.h"
class clientRequestContent
{
public:
	void loadFileInContent(std::string dowloadPath, std::string htmlFile, std::string *content, uint16_t *codereq, bool getfile);
	
	void sendContentToUser(SOCKET UserConnection, uint16_t codereq, std::string content, uint16_t sizeJson, bool getfile, json jsonFIle);
	
};
