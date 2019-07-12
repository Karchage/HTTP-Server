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
	// ===============================================================================
	bool closeConnection = false;
	//char welcomeMsg[255];
	const int BUFFER_SIZE = 1024;
	char bufferFile[BUFFER_SIZE];
	char fileRequested[FILENAME_MAX]; //Имя файла
	std::ofstream file;
	int codeFile = 404;
	const int trueFile = 200;
	const int badFile = 404;
	long fileRequestedsize = 0;
	
	do
	{
		int fileDownloaded = 0;
		//memset(fileRequested, 0, FILENAME_MAX);
		std::cout << "File name : ";
		std::cin.getline(fileRequested, FILENAME_MAX);

		int sendInf = send(Connection, fileRequested, FILENAME_MAX, NULL);
		if (sendInf == 0 || sendInf == -1)
		{
			closeConnection = true;
			break;
		}
		int byRecv = recv(Connection, (char*)&codeFile, sizeof(int), NULL);
		if (byRecv == 0 || byRecv == -1)
		{
			closeConnection = true;
			break;
		}
		if (codeFile == 200)
		{
			byRecv = recv(Connection, (char*)&fileRequestedsize, sizeof(long), NULL);
			if (byRecv == 0 || byRecv == -1)
			{
				closeConnection = true;
				break;
			}
			file.open(fileRequested, std::ios::binary | std::ios::trunc);

			do
			{
				memset(bufferFile, 0, BUFFER_SIZE);
				byRecv = recv(Connection, bufferFile, BUFFER_SIZE, NULL);
				if (byRecv == 0 || byRecv == -1)
				{
					closeConnection = true;
					break;
				}
				file.write(bufferFile, byRecv);
				fileDownloaded += byRecv;
			} 
			while (fileDownloaded < fileRequestedsize);
			file.close();
		}
		else if (codeFile == 404)
		{
			std::cout << "File not found" << std::endl;
		}
	} while (!closeConnection);

	system("pause");
	return 0;

}
