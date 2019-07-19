﻿#include "pch.h"
#include "httpServer.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

using json = nlohmann::json;
#pragma comment (lib, "Ws2_32.lib")
typedef struct
{
	SOCKET *sok;
	char * inf;
}getInf;

uint8_t dontblockSocket(SOCKET fd)
{
	u_long flags;
#if defined(O_NONBLOCK)
	if (-1 == (flags = fcntl(fd, F_GETL, NULL)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctlsocket(fd, FIONBIO, &flags);
#endif
}
class clientRequestContent
{
public:
	void loadFileInContent(std::string dowloadPath, std::string htmlFile, std::string *content, uint16_t *codereq, bool getfile)
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
	void sendContentToUser(SOCKET UserConnection, uint16_t codereq, std::string content, uint16_t sizeJson, bool getfile, json jsonFIle)
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
};
void ClientHandler(void * inform)
{
	clientRequestContent Client;
	getInf * ptr = (getInf *)inform;
	boost::property_tree::ptree tree;
	boost::property_tree::read_xml("./settings.xml", tree);
	std::string dowloadPath = tree.get <std::string >("server.server_info.dowloadPath");
	json jsonFIle;
	bool getfile = true;
	uint16_t codereq;
	std::stringstream iss(ptr->inf);
	SOCKET UserConnection = *ptr->sok;
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
class webServer
{
public:
	std::string ipInform;
	uint16_t portInform;
	void webServerRun()
	{
		WSAData wsaData;
		WORD Version = MAKEWORD(2, 1); //дает версию 
		if (WSAStartup(Version, &wsaData) != 0) //Грузим библ
		{
			std::cout << "Error: Library not loaded" << std::endl;
		}
		else std::cout << "Library connected" << std::endl;

		SOCKADDR_IN adrSock; //хранение адреса

		inet_pton(AF_INET, ipInform.c_str(), &(adrSock.sin_addr));

		adrSock.sin_port = htons(portInform);
		adrSock.sin_family = AF_INET;

		SOCKET lsock = socket(AF_INET, SOCK_STREAM, NULL);
		if (lsock == -1)
		{
			std::cout << "Error : Invalid socket" << std::endl;
		}
		bind(lsock, (SOCKADDR*)&adrSock, sizeof(adrSock)); //назначение


		dontblockSocket(lsock);
		listen(lsock, SOMAXCONN); // второе значение скок запрос 
		getPoll(lsock);
	}
private:
	void getPoll(SOCKET lsock)
	{
		std::set<SOCKET> userConnectionSockets;
		struct pollfd SetUser[1024];
		SetUser[0].fd = lsock;
		SetUser[0].events = POLLIN;

		while (true)
		{
			uint16_t Index = 1;
			for (auto Iter = userConnectionSockets.begin(); Iter != userConnectionSockets.end(); Iter++)
			{
				SetUser[Index].fd = *Iter;
				SetUser[Index].events = POLLIN;
				Index++;
			}

			uint16_t SetSize = 1 + userConnectionSockets.size();
			WSAPoll(SetUser, SetSize, -1);

			for (uint16_t i = 0; i < SetSize; i++)
			{
				if (SetUser[i].revents & POLLIN)
				{
					if (i != 0)
					{
						char Buffer[1024];
						uint16_t RecvSize = recv(SetUser[i].fd, Buffer, sizeof(Buffer), NULL);
						Buffer[RecvSize] = '\0';
						if ((RecvSize == 0) && (errno != EAGAIN))
						{
							shutdown(SetUser[i].fd, SD_BOTH);
							closesocket(SetUser[i].fd);
							userConnectionSockets.erase(SetUser[i].fd);
						}
						else if (RecvSize > 0)
						{
							std::cout << "Request from: \n" << Buffer;
							getInf inform;
							inform.sok = &SetUser[i].fd;
							inform.inf = Buffer;
							CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(&inform), NULL, NULL);
						}
					}
					else
					{
						SOCKET userConnectionSocket = accept(lsock, 0, 0);
						dontblockSocket(userConnectionSocket);
						userConnectionSockets.insert(userConnectionSocket);
					}
				}
			}
		}
	}

};




int main(int argc, char* argv[])
{
	webServer server;

	boost::property_tree::ptree tree;
	boost::property_tree::read_xml("./settings.xml", tree);
	//constexpr uint16_t SET_SIZE = 1024;
	server.ipInform = tree.get <std::string >("server.server_info.ip");
	server.portInform = tree.get <uint16_t>("server.server_info.port");

	if (server.ipInform != "" && server.portInform != NULL)
	{
		server.webServerRun();
	}


	
	
	return 0;
}

