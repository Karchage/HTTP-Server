#include "pch.h"
#include "httpServer.h"

//Define
#define dowloadPath  ".\\Download\\"
#define SET_SIZE 1024
#define BUFF_SIZE 1024
#define IP_INFORM "127.0.0.1"
#define PORT_INFORM 8015



using json = nlohmann::json;
#pragma comment (lib, "Ws2_32.lib")

typedef struct 
{
	SOCKET *sok;
	char * inf;
}getInf;



int dontblockSocket(int fd)
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

void ClientHandler(void * inform)
{
	getInf * ptr = (getInf *)inform;

	json j;

	bool getfile = true;
	int codereq;


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
		send(UserConnection, response.str().c_str(), response.str().length(), NULL);
	}
	else
	{
		response << "HTTP/1.1" << codereq << " OK \r\n"
			<< "Content-Length: "
			<< content.size()
			<< "\r\n\r\n"
			<< content;
		int size = response.str().length() + content.size();
		send(UserConnection, response.str().c_str(), size, NULL);
	}
}
	

int main(int argc, char* argv[])
{

	std::set<int> userConnectionSockets;


	WSAData wsaData;
	WORD Version = MAKEWORD(2, 1); //дает версию 
	if (WSAStartup(Version, &wsaData) != 0) //Грузим библ
	{
		std::cout << "Error: Library not loaded" << std::endl;
		exit(1);
	}
	else std::cout << "Library connected" << std::endl;

	SOCKADDR_IN adrSock; //хранение адреса
	static const char LOCAL_HOST[] = IP_INFORM;
	int sizeAddr = sizeof(adrSock);
	inet_pton(AF_INET, LOCAL_HOST, &(adrSock.sin_addr));
	adrSock.sin_port = htons(PORT_INFORM);
	adrSock.sin_family = AF_INET;

	SOCKET lsock = socket(AF_INET, SOCK_STREAM, NULL);
	if (lsock == -1)
	{
		std::cout << "Error : Invalid socket" << std::endl;
	}
	bind(lsock, (SOCKADDR*)&adrSock, sizeof(adrSock)); //назначение


	dontblockSocket(lsock);
	listen(lsock, SOMAXCONN); // второе значение скок запрос 
	/*
	typedef struct pollfd {

    SOCKET  fd;
    SHORT   events;
    SHORT   revents;

	} WSAPOLLFD, *PWSAPOLLFD, FAR *LPWSAPOLLFD;*/
	struct pollfd Set[SET_SIZE];
	Set[0].fd = lsock;
	Set[0].events = POLLIN;

	while (true)
	{
		unsigned int Index = 1;
		for (auto Iter = userConnectionSockets.begin(); Iter != userConnectionSockets.end();Iter++)
		{
			Set[Index].fd = *Iter;
			Set[Index].events = POLLIN;
			Index++;
		}
		unsigned int SetSize = 1 + userConnectionSockets.size();
		WSAPoll(Set, SetSize, -1);
		
		for(unsigned int i = 0; i < SetSize; i++)
		{
			if (Set[i].revents & POLLIN)
			{
				if (i != 0)
				{
					char Buffer[BUFF_SIZE];
					int RecvSize = recv(Set[i].fd, Buffer, BUFF_SIZE, NULL);
					Buffer[RecvSize] = '\0';
					if ((RecvSize == 0) && (errno != EAGAIN))
					{
						shutdown(Set[i].fd, SD_BOTH);
						closesocket(Set[i].fd);
						userConnectionSockets.erase(Set[i].fd);
					}
					else if (RecvSize >0)
					{
						std::cout << "Request from: \n" << Buffer;
						getInf inform;
						inform.sok = &Set[i].fd;
						inform.inf = Buffer;
						CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(&inform), NULL, NULL);
					}
				}
				else 
				{
					int userConnectionSocket = accept(lsock,0,0);
					dontblockSocket(userConnectionSocket);
					userConnectionSockets.insert(userConnectionSocket);
				}
			}
		}
	}
	return 0;
}
