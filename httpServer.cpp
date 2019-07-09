#include "pch.h"
#include <iostream>
#include <WinSock2.h>

int main(int argc, char* argv[])
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1); //дает версию 
	if(WSAStartup(DLLVersion, &wsaData) != 0) //Грузим библ
	{
		std::cout << "Escape, error: lib not load" << std::endl;
		exit(1);
	}

	SOCKADDR_IN adrSock; //хранение адреса
	int sizeAddr = sizeof(adrSock);
	adrSock.sin_addr.s_addr = inet_addr("127.0.0.1"); //inet_addr("128.0.0.1")
	adrSock.sin_port = htons(8015);
	adrSock.sin_family = AF_INET; // хз что тут надо 
								  //AF_INET6

	SOCKET socnasok = socket(AF_INET, SOCK_STREAM, NULL);
	bind(socnasok, (SOCKADDR*)&adrSock,sizeof(adrSock));
	listen(socnasok, SOMAXCONN); // второе значение хау мач запрос 

	SOCKET newUserConnection;
	newUserConnection = accept(socnasok, (SOCKADDR*)&adrSock, &sizeAddr);

	if (newUserConnection == 0) // проверка коннект
	{
		std::cout << "Error connection" << std::endl;
	}

	return 0;
}
