#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <fstream>
#pragma comment (lib, "Ws2_32.lib")
int main(int argc, char* argv[])
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1); //дает версию 
	if (WSAStartup(DLLVersion, &wsaData) != 0) //Грузим библ
	{
		std::cout << "Error : Library not loaded" << std::endl;
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

	SOCKET Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&adrSock, sizeof(adrSock)) != 0)
	{
		std::cout << "Error : Connection fail";
		return 1;
	}
	std::cout << "Conected !!";
	char wmsg[64];
	recv(Connection, wmsg, sizeof(wmsg), NULL); //get msg fr serv
	std::cout << wmsg;


	char fileName[FILENAME_MAX];
	std::ofstream file;
	const int buffer = 1024;
	char bufferFile[buffer];
	bool closeConnect = false;
	long fileSize = 0;
	do
	{
		int fileDownloaded = 0;
		memset(fileName, 0, FILENAME_MAX);
		std::cout << " File name:" << std::endl; // Тут торчит
		std::cin.getline(fileName, FILENAME_MAX);

		send(Connection, fileName, FILENAME_MAX, NULL);

		int byRecv = recv(Connection, (char*)&fileSize, sizeof(long), NULL);
		if (byRecv == 0)
		{
			closeConnect = true;
			break;
		}

		file.open(fileName, std::ios::binary, std::ios::trunc);
		do
		{
			memset(bufferFile, 0, buffer);
			byRecv = recv(Connection, bufferFile, buffer, NULL);
			if (byRecv == 0)
			{
				closeConnect = true;
				break;
			}
			file.write(bufferFile, byRecv);
			fileDownloaded += byRecv;

		} while (fileDownloaded < fileSize);

	} while (!closeConnect);


	system("pause");
	return 0;

}
