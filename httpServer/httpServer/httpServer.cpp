#include "pch.h"
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>
#pragma warning (disable : 4996)
#pragma comment (lib, "Ws2_32.lib")

SOCKET Connection[100];
int counter = 0;
void ClientHandler(int index)
{
	const int max_client_buffer_size = 60000;
	char buf[max_client_buffer_size];

	recv(Connection[index], buf, max_client_buffer_size, 0);
	int del;
	int del2;
	std::string iss = buf;
	std::string content;
	int cont;
	std::string htmlFile;

	//char gete[] = "GET /123214.txt HTTP/1.1\r\n";
	del = iss.find("\r\n") ;
	htmlFile = iss.substr(0, del);
	del = htmlFile.find("HTTP");
	del2 = htmlFile.find("/") + 1;
	htmlFile = htmlFile.substr(del2, del - del2);

	std::stringstream response; // сюда будет записываться ответ клиенту

	std::ifstream f(htmlFile);
	
	std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	content = str;
		

	f.close();
		// Формируем весь ответ вместе с заголовками
		response << "HTTP/1.1 200 OK \r\n"
			<< "Content-Length: "
			<< content.size()
			<< "\r\n\r\n"
			<< content;
		int size = response.str().length() + content.size();
		// Отправляем ответ клиенту с помощью функции send*/
		send(Connection[index], response.str().c_str(), size, NULL);

		
		closesocket(Connection[index]);
}
int main(int argc, char* argv[])
{

	//std::set<int> SlaveSockets;
	

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
	
	
	
	//set_nonblock(lsock);
	listen(lsock, SOMAXCONN); // второе значение скок запрос 
	SOCKET userConnection;
	for (int i = 0; i < 100; i++)
	{
		userConnection = accept(lsock, (SOCKADDR*)&adrSock, &sizeAddr);

		if (userConnection == 0)
		{
			std::cout << "Error" << std::endl;
		}
		else
		{
			std::cout << "Client connected !" << std::endl;
			//char wmsg[64] = "Welcome to http-server \n";
			//send(userConnection, wmsg, sizeof(wmsg), NULL); //  SEND Welcome msg
			
			Connection[i] = userConnection;
			counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
		}
	}
	
	return 0;
}


/*const int trueFile = 200;
	const int badFile = 404;

	char filename[FILENAME_MAX];
	const int bufer = 1024;
	char bufferFile[bufer];
	bool closeConnection = false;
	std::ifstream file;

	do
	{
		//memset(filename, 0, FILENAME_MAX);
		int byRecv = recv(Connection[index], filename, FILENAME_MAX, NULL); //Имя файла
		if (byRecv == 0 || byRecv == -1)
		{
			closeConnection = true;
		}

		file.open(filename, std::ios::binary); //Открываем поток файла в бинарке
		if (file.is_open())
		{
			int sendInf = send(Connection[index], (char*)&trueFile, sizeof(int), NULL); //нахождение файла сделать если не найден прерывать
			if (sendInf == 0 || sendInf == -1)
			{
				closeConnection = true;
			}

			file.seekg(0, std::ios::end); // Размер файла
			long fileSize = file.tellg(); // Текущая позиция =  размер файла
			sendInf = send(Connection[index], (char*)&fileSize, sizeof(long), NULL); // Тож проверку запилить
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
					sendInf = send(Connection[index], bufferFile, file.gcount(), NULL);
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
			int sendInf = send(index, (char*)&badFile, sizeof(int), NULL);
			if (sendInf == 0 || sendInf == -1)
			{
				closeConnection = true;
			}
		}

	} while (!closeConnection);*/
