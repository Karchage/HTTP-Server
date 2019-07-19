#include "pch.h"
#include "webServer.h"
#include "userConnect.h"

uint8_t webServer::dontblockSocket(SOCKET fd)
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
void webServer::getPoll(SOCKET lsock)
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
						userConnect User;
						User.sok = &SetUser[i].fd;
						User.inf = Buffer;
						CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)User.ClientHandler, (void *)&User, NULL, NULL);
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
void webServer::webServerRun()
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
