#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
int main(int argc, char* argv[])
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1); //дает версию 
	if(WSAStartup(DLLVersion, &wsaData) != 0) //Грузим библ
	{
		std::cout << "Escape, error: lib not load" << std::endl;
		exit(1);
	}
	else std::cout << "Lib connected !!!" << std::endl;

	SOCKADDR_IN adrSock; //хранение адреса
	static const char LOCAL_HOST[] = "127.0.0.1";
	int sizeAddr = sizeof(adrSock);
	//adrSock.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	inet_pton(AF_INET, LOCAL_HOST, &(adrSock.sin_addr));
	adrSock.sin_port = htons(8015);
	adrSock.sin_family = AF_INET;

	SOCKET socnasok = socket(AF_INET, SOCK_STREAM, NULL);
	bind(socnasok, (SOCKADDR*)&adrSock,sizeof(adrSock)); //назначение
	listen(socnasok, SOMAXCONN); // второе значение хау мач запрос 

	SOCKET newUserConnection;
	newUserConnection = accept(socnasok, (SOCKADDR*)&adrSock, &sizeAddr);

	if (newUserConnection == 0) // проверка коннект
	{
		std::cout << "Error connection" << std::endl;
	}
	else
	{
		std::cout << "Client connected !" << std::endl;
		char wmsg[64] = "Welcome to http-server \n";
		send(newUserConnection, wmsg, sizeof(wmsg), NULL); //  SEND Welcome msg
		
	}

	return 0;
}
