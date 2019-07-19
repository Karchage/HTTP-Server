#include "pch.h"
#include "httpServer.h"
#include "clientRequestContent.h"

void clientRequestContent::loadFileInContent(std::string dowloadPath, std::string htmlFile, std::string *content, uint16_t *codereq, bool getfile)
{
	std::ifstream fileload(dowloadPath + htmlFile);

	if (fileload.good())
	{
		std::string str((std::istreambuf_iterator<char>(fileload)), std::istreambuf_iterator<char>());
		*content = str;
		*codereq = 200;
	}
	else if (getfile == true)
	{
		*codereq = 404;
	}
	fileload.close();
}
void clientRequestContent::sendContentToUser(SOCKET UserConnection, uint16_t codereq, std::string content, uint16_t sizeJson, bool getfile, json jsonFIle)
{
	std::stringstream response;
	if (getfile == false)
	{
		response << "HTTP/1.1 200 OK \r\n"
			<< "Content-Type : application/json" << "\r\n"
			<< "Content-Length: "
			<< sizeJson
			<< "\r\n\r\n"
			<< jsonFIle;
		send(UserConnection, response.str().c_str(), response.str().length(), NULL);
	}
	else
	{
		response << "HTTP/1.1" << codereq << " OK \r\n"
			<< "Content-Length: "
			<< content.size()
			<< "\r\n\r\n"
			<< content;
		uint32_t size = response.str().length() + content.size();
		send(UserConnection, response.str().c_str(), size, NULL);
	}
}