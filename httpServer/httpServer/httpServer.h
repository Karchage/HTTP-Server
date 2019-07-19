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
#include <filesystem>
#include "nlohmann/json.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include "pch.h"
using json = nlohmann::json;
#pragma comment (lib, "Ws2_32.lib")