#include "pch.h"
#include "httpServer.h"
#include "webServer.h"

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

