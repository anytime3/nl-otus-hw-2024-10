#include <string>
#include <iostream>
#include <cstdint>
#include <memory>
#include "server.h"
#include "builder.h"
using namespace std;

void printHelp()
{
	cout << "Usage: join_server <port>" << endl;
	cout << "\tport - tcp port to use" << endl;
}

int main(int argc, char** argv)
{
	std::locale::global(std::locale(""));
	if (argc <= 1 || string(argv[1]) == "--help")
	{
		printHelp();
		return 0;
	}

	uint16_t port = stoi(string(argv[1]));

	unique_ptr<Server> server = Builder::build(port);
	server->start();

	string s; // any input stops the server
	cin >> s;

	server->stop();

    return 0;
}