#include "pch.h"
#include "httpServer.h"
#include "clientRequestContent.h"

void clientRequestContent::loadFileInContent(std::string dowloadPath, std::string htmlFile, std::string *content, uint16_t *codereq, bool getfile)
{
	if (htmlFile == "help.html")
	{
		dowloadPath = ".//web//";
	}
	std::ifstream fileload(dowloadPath + htmlFile,std::ios::binary);
	
	if (fileload.good())
	{
		std::string str((std::istreambuf_iterator<char>(fileload)), std::istreambuf_iterator<char>());
		*content = str;
		*codereq = 200;
	}
	else
	{
		*codereq = 404;
	}
	fileload.close();
}
void clientRequestContent::sendContentToUser(SOCKET UserConnection, uint16_t codereq, std::string content, uint16_t sizeJson,
	bool getfile, json jsonFIle, std::string dowloadPath, std::string htmlFile)
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
		response << "HTTP/1.1 " << codereq << " OK \r\n"
			<< "Content-Type : "<< getFileType(dowloadPath,htmlFile) << "\r\n"
			<< "Content-Length: "
			<< content.size()
			<< "\r\n\r\n"
			<< content;
		send(UserConnection, response.str().c_str(), response.str().length(), NULL);
	}
}
std::string clientRequestContent :: getFileType(std::string dowloadPath, std::string htmlFile)
{
	std::string contentType;
	std::map<std::string, std::string> fileType = { {".txt","text/plain"},{".png","image/png"},
	{".zip","application/zip"},{".html","text/html"},{".rar","application/x-rar-compressed"},{".7z","application/x-7z-compressed"} };

	for (auto &p : std::experimental::filesystem::directory_iterator(std::experimental::filesystem::path() = dowloadPath))
	{
		if (std::experimental::filesystem::is_regular_file(p))
		{
			if (htmlFile == std::experimental::filesystem::path(p).filename().string())
			{
				contentType = fileType[std::experimental::filesystem::path(p).extension().string()];
			}

		}
	}
	return contentType;
}