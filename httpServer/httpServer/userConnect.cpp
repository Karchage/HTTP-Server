#include "pch.h"
#include "httpServer.h"
#include "clientRequestContent.h"
#include "userConnect.h"



void userConnect::ClientHandler(userConnect User)
{
	clientRequestContent Client;
	//getInf * ptr = (getInf *)inform;
	boost::property_tree::ptree tree;
	boost::property_tree::read_xml("./settings.xml", tree);
	std::string dowloadPath = tree.get <std::string >("server.server_info.dowloadPath");
	json jsonFIle;
	bool getfile = true;
	uint16_t codereq;
	std::stringstream iss(User.inf);
	SOCKET UserConnection = *User.sok;
	std::vector <std::string> vectorParsing((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	std::string htmlFile;
	std::string content = "<h1>404</h1>";
	std::stringstream response_body;
	if (vectorParsing.size() >= 3 && vectorParsing[0] == "GET")
	{
		htmlFile = vectorParsing[1];
		if (htmlFile == "/" || htmlFile == "/files")
		{


			for (auto &p : std::experimental::filesystem::directory_iterator(std::experimental::filesystem::path() = "./Download"))
			{
				if (std::experimental::filesystem::is_regular_file(p))
				{
					jsonFIle[std::experimental::filesystem::path(p).filename().string()] = { "Size" ,std::to_string(std::experimental::filesystem::file_size(p)) };

				}
			}
			response_body << jsonFIle;
			getfile = false;
		}
		if (htmlFile != "" || htmlFile != "/")
		{
			htmlFile.erase(htmlFile.find('/'), 1);
		}
	}

	Client.loadFileInContent(dowloadPath, htmlFile, &content, &codereq, getfile);
	Client.sendContentToUser(UserConnection, codereq, content, response_body.str().length(), getfile, jsonFIle);
}