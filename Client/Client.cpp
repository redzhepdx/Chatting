//CLIENT
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


SOCKET Connection;

bool SendInt(int _int) {
	if (send(Connection, (char*)&_int, sizeof(int), NULL) == SOCKET_ERROR) // if int failed to send due to connection issue
		return false;
	return true;
}

bool GetInt(int &_int) {
	if (recv(Connection, (char*)&_int, sizeof(int), NULL) == SOCKET_ERROR)
		return false;
	return true;
}

bool SendString(std::string &_string) {
	int buffer_size = _string.size();

	if (!SendInt(buffer_size))
		return false;

	if (send(Connection, _string.c_str(), buffer_size, NULL) == SOCKET_ERROR) 
		return false;

	return true;
}

bool GetString(std::string &_string) {
	int buffer_size;
	if (!GetInt(buffer_size)) 
		return false;

	char * buffer = new char[buffer_size + 1];
	
	buffer[buffer_size] = '\0';

	if (recv(Connection, buffer, buffer_size, NULL) == SOCKET_ERROR)
		return false;

	_string = buffer;
	delete[] buffer;
	
	return true;
}

void ClientThread() {
	
	while (true) {

		std::string msg;
		if (!GetString(msg))
			break;
		
		std::cout << msg << std::endl;

		if (msg.compare("clear") == 0) {
			system("cls");
		}
	}

	std::cout << "Lost Connection to the Server." << std::endl;
	closesocket(Connection);
}

//First send integer which will hold the length of the string
//Send our string

int main(int argc, char** argv)
{
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
	}

	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);

	//Startup Configuration
	//if WSAStartup return anything other than 0, then it is an error
	if (WSAStartup(DllVersion, &wsaData) != 0) {
		MessageBox(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	
	//Client Config
	SOCKADDR_IN addr; //Address to be binded to our Connection socket
	int addr_len = sizeof(addr); //Need sizeoaddr for the connect function
	addr.sin_addr.s_addr = inet_addr(IP.c_str()); //Address = localhost (this pc)
	addr.sin_port = htons(PORT); //Port = 1111
	addr.sin_family = AF_INET; //IPv4 Socket

	//Connection to server
	Connection = socket(AF_INET, SOCK_STREAM, NULL); //Set Connection Socket
	if (connect(Connection, (SOCKADDR*)&addr, addr_len) != 0) { // If we are unable to connect
		MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	std::cout << "Connected.. Waiting For Login Queue" << std::endl;
	bool flag = false;

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientThread, NULL, NULL, NULL); //Create Client Thread

	std::string buffer;
	while (true) {
		std::getline(std::cin, buffer);
		if (!SendString(buffer))
			break;
		Sleep(10);
	}

	system("Pause");
	return 0;
}