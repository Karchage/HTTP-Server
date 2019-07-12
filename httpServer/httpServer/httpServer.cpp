#include "pch.h"
#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
int main(int argc, char* argv[])
{
	SOCKET connectionsUser[128];
	WSAData wsaData;
	WORD Version = MAKEWORD(2, 1); //дает версию 
	if(WSAStartup(Version, &wsaData) != 0) //Грузим библ
	{
		std::cout << "Error: Library not loaded" << std::endl;
		exit(1);
	}
	else std::cout << "Library connected" << std::endl;

	SOCKADDR_IN adrSock; //хранение адреса
	static const char LOCAL_HOST[] = "127.0.0.1";
	int sizeAddr = sizeof(adrSock);
	//adrSock.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	inet_pton(AF_INET, LOCAL_HOST, &(adrSock.sin_addr));
	adrSock.sin_port = htons(8015);
	adrSock.sin_family = AF_INET;

	SOCKET lsock = socket(AF_INET, SOCK_STREAM, NULL);
	if (lsock == -1)
	{
		std::cout << "Error : Invalid socket" << std::endl;
	}
	bind(lsock, (SOCKADDR*)&adrSock,sizeof(adrSock)); //назначение
	listen(lsock, SOMAXCONN); // второе значение скок запрос 

	for (int i = 0; i < 128; i++)
	{
		SOCKET newUserConnection;
		newUserConnection = accept(lsock, (SOCKADDR*)&adrSock, &sizeAddr);
		if (newUserConnection == 0) // проверка коннект
		{
			std::cout << "Error connection" << std::endl;
		}
		else
		{
			std::cout << "Client connected !" << std::endl;
			char wmsg[64] = "Welcome to http-server \n";
			connectionsUser[i] = newUserConnection;
			send(newUserConnection, wmsg, sizeof(wmsg), NULL); //  SEND Welcome msg

		}
		//==========================================================================================


		//signal code
		const int trueFile = 200;
		const int badFile = 404;

		char filename[FILENAME_MAX];
		const int bufer = 1024;
		char bufferFile[bufer];
		bool closeConnection = false;
		std::ifstream file;

		do
		{
			//memset(filename, 0, FILENAME_MAX);
			int byRecv = recv(newUserConnection, filename, FILENAME_MAX, NULL); //Имя файла
			if (byRecv == 0 || byRecv == -1)
			{
				closeConnection = true;
			}

			file.open(filename, std::ios::binary); //Открываем поток файла в бинарке
			if (file.is_open())
			{
				int sendInf = send(newUserConnection, (char*)&trueFile, sizeof(int), NULL); //нахождение файла сделать если не найден прерывать
				if (sendInf == 0 || sendInf == -1)
				{
					closeConnection = true;
				}

				file.seekg(0, std::ios::end); // Размер файла
				long fileSize = file.tellg(); // Текущая позиция =  размер файла
				sendInf = send(newUserConnection, (char*)&fileSize, sizeof(long), NULL); // Тож проверку запилить
				if (sendInf == 0 || sendInf == -1)
				{
					closeConnection = true;
				}
				file.seekg(0, std::ios::beg); // Сдвигаем на начало
				do
				{
					file.read(bufferFile, bufer);
					if (file.gcount() > 0)
					{
						sendInf = send(newUserConnection, bufferFile, file.gcount(), NULL);
					}
					if (sendInf == 0 || sendInf == -1)
					{
						closeConnection = true;
						break;
					}

				} while (file.gcount() > 0);
				file.close();
			}
			else
			{
				int sendInf = send(newUserConnection, (char*)&badFile, sizeof(int), NULL);
				if (sendInf == 0 || sendInf == -1)
				{
					closeConnection = true;
				}
			}

		} while (!closeConnection);
	}
	

	return 0;
}
