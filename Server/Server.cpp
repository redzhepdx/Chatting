//SERVER

#include "ServerAgents.h"
#include <conio.h>

int main(int argc, char** argv) {

	std::string IP;
	int PORT;

	if (argc != 0) {
		IP = argv[1];
		PORT = std::atoi(argv[2]);
		std::cout << "SERVER IP : " << IP << std::endl;
		std::cout << "PORT : " << PORT << std::endl;
	}
	else {
		IP = "127.0.0.1";
		PORT = 1111;
		std::cout << "SERVER IP : " << IP << std::endl;
		std::cout << "PORT : " << PORT << std::endl;
	}

	std::unique_ptr<ServerController> server = std::make_unique<ServerController>(IP, PORT);

	server->AddRoom("Room-1");
	server->AddRoom("Room-2");
	server->AddRoom("Room-3");
	server->AddRoom("Room-4");

	for (int i = 0; i < SERVER_CAPACITY; i++) {
		server->ListenForNewConnection();
	}
	system("pause");
	return 0;
}