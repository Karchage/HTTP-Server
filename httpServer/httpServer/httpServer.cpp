#include "pch.h"
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>
#include <filesystem>
#include "nlohmann/json.hpp"
#define dowloadPath  ".\\Download\\"
#define POOL_SIZE 1024
using json = nlohmann::json;
#pragma warning (disable : 4996)
#pragma comment (lib, "Ws2_32.lib")

typedef struct 
{
	SOCKET *sok;
	char * inf;
}getInf;



#if defined (WIN32)
static inline int poll(struct pollfd *pfd, int nfds, int timeout) { return WSAPoll(pfd, nfds, timeout); }
#endif

int set_nonblock(int fd)
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
SOCKET Connection[100];
int counter = 0;
//class pollfd Set[POOL_SIZE];
void ClientHandler(void * inform)
{
	getInf * ptr = (getInf *)inform;
	
	json j;
	//const int max_client_buffer_size = 60000;
	//char buf[max_client_buffer_size];
	bool getfile = true;
	int codereq;
	//int result = recv(UserConnection, buf, max_client_buffer_size, 0);
	
	std::stringstream iss(ptr->inf);
	SOCKET UserConnection = *ptr->sok;
	std::vector <std::string> vectorParsing((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	std::string htmlFile;
	std::string content = "<h1>404</h1>";
	std::stringstream response_body;
	if(vectorParsing.size() >= 3 && vectorParsing[0] == "GET")
	{
		htmlFile = vectorParsing[1];
		if (htmlFile == "/" || htmlFile == "/files" )
		{


			for (auto &p : std::experimental::filesystem::directory_iterator(std::experimental::filesystem::path() = "./Download"))
			{
				if (std::experimental::filesystem::is_regular_file(p))
				{
					j[std::experimental::filesystem::path(p).filename().string()] = {"Size" ,std::to_string(std::experimental::filesystem::file_size(p)) } ;

				}
			}
			response_body << j;
			getfile = false;
		}
		if (htmlFile != "" || htmlFile != "/")
		{
			htmlFile.erase(htmlFile.find('/'), 1);
		}
	}
	//===============================================================================================
	std::stringstream response; // сюда будет записываться ответ клиенту
	
	std::ifstream f(dowloadPath+htmlFile);

	if (f.good())
	{
		std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		content = str;
		codereq = 200;
	}
	else if(getfile == true)
	{
		codereq = 404;
	}
	


	f.close();
	// Формируем весь ответ вместе с заголовками
	//=========================================================
	if (getfile == false)
	{
		response << "HTTP/1.1 200 OK \r\n"
			//<< "Content-Type : application/json"<< "\r\n"
			<< "Content-Length: "
			<< response_body.str().length()
			<< "\r\n\r\n"
			<< response_body.str();
		send(UserConnection, response.str().c_str(), response.str().length(), NULL);
	}
	else
	{
		response << "HTTP/1.1" << codereq<<" OK \r\n"
			<< "Content-Length: "
			<< content.size()
			<< "\r\n\r\n"
			<< content;
		int size = response.str().length() + content.size();
		send(UserConnection, response.str().c_str(), size, NULL);
	}
	
	
	
	// Отправляем ответ клиенту с помощью функции send*/
	


	//closesocket(UserConnection);
}
int main(int argc, char* argv[])
{

	std::set<int> SlaveSockets;


	WSAData wsaData;
	WORD Version = MAKEWORD(2, 1); //дает версию 
	if (WSAStartup(Version, &wsaData) != 0) //Грузим библ
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
	bind(lsock, (SOCKADDR*)&adrSock, sizeof(adrSock)); //назначение



	set_nonblock(lsock);
	listen(lsock, SOMAXCONN); // второе значение скок запрос 
	struct pollfd Set[POOL_SIZE];
	Set[0].fd = lsock;
	Set[0].events = POLLIN;

	while (true)
	{
		unsigned int Index = 1;
		for (auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end();Iter++)
		{
			Set[Index].fd = *Iter;
			Set[Index].events = POLLIN;
			Index++;
		}
		unsigned int SetSize = 1 + SlaveSockets.size();
		WSAPoll(Set, SetSize, -1);
		
		for(unsigned int i = 0; i < SetSize; i++)
		{
			if (Set[i].revents & POLLIN)
			{
				if (i != 0)
				{
					char Buffer[1024];
					int RecvSize = recv(Set[i].fd, Buffer, 1024, NULL);
					Buffer[RecvSize] = '\0';
					if ((RecvSize == 0) && (errno != EAGAIN))
					{
						shutdown(Set[i].fd, SD_BOTH);
						closesocket(Set[i].fd);
						SlaveSockets.erase(Set[i].fd);
					}
					else if (RecvSize >0)
					{
							/*counter = counter + 1;
						std::cout << "new request "<<counter<<" \n";
						json j;
						//const int max_client_buffer_size = 60000;
						//char buf[max_client_buffer_size];
						bool getfile = true;
						int codereq;
						//int result = recv(UserConnection, buf, max_client_buffer_size, 0);

						std::stringstream iss(Buffer);
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
										j[std::experimental::filesystem::path(p).filename().string()] = { "Size" ,std::to_string(std::experimental::filesystem::file_size(p)) };

									}
								}
								response_body << j;
								getfile = false;
							}
							if (htmlFile != "" || htmlFile != "/")
							{
								htmlFile.erase(htmlFile.find('/'), 1);
							}
						}
						//===============================================================================================
						std::stringstream response; // сюда будет записываться ответ клиенту

						std::ifstream f(dowloadPath + htmlFile);

						if (f.good())
						{
							std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
							content = str;
							codereq = 200;
						}
						else if (getfile == true)
						{
							codereq = 404;
						}



						f.close();
						// Формируем весь ответ вместе с заголовками
						//=========================================================
						if (getfile == false)
						{
							response << "HTTP/1.1 200 OK \r\n"
								//<< "Content-Type : application/json"<< "\r\n"
								<< "Content-Length: "
								<< response_body.str().length()
								<< "\r\n\r\n"
								<< response_body.str();
							send(Set[i].fd, response.str().c_str(), response.str().length(), NULL);
						}
						else
						{
							response << "HTTP/1.1" << codereq << " OK \r\n"
								<< "Content-Length: "
								<< content.size()
								<< "\r\n\r\n"
								<< content;
							int size = response.str().length() + content.size();
							send(Set[i].fd, response.str().c_str(), size, NULL);
						}*/
						//std::string info = Buffer;
						
						getInf inform;
						inform.sok = &Set[i].fd;
							inform.inf = Buffer;
						
						CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(&inform), NULL, NULL);
					}
				}
				else 
				{
					int SlaveSocket = accept(lsock,0,0);
					set_nonblock(SlaveSocket);
					SlaveSockets.insert(SlaveSocket);
				}
			}
		}
	}


	/*SOCKET userConnection;
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
			
		}
	}*/

	

	return 0;
}
