#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef SERVER_AGENTS_
#define SERVER_AGENTS_

#include<vector>
#include<iostream>
#include<unordered_map>
#include<string>
#include<memory>
#include<thread>
#include<mutex>
#include<exception>
#include<stdio.h>

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>

#define MAX_CAPACITY 100
#define SERVER_CAPACITY 1000
#define CL_MSG std::string("clear")
#define START_CHAT_MSG(X, Y) "Hey : " + X + " Welcome To : " + Y
#define SEND_MSG(F, CI, SMSG) F(CI, SMSG)
#define SEND_CL_MSG(F, CI) F(CI, CL_MSG) 



class ChatRoom;

class Client {
public:
	Client();
	Client* copy();

	int glob_index; //Client's Global Index
	int curr_room; //Client's Room's index
	int room_index; //Client's Index inside Room
	int connection_index; //Client's Connection Index
	bool isInRoom = false;
	std::string name; //Name Of Client
	std::string id; //Entrance Id Of Client
	std::string pw; //Password of Client
	
};

class ChatRoom{
public:
	ChatRoom();
	ChatRoom(std::string name);
	
	int getIndex(); //Get Room Index 
	void setIndex(int index); //Set Room Index
	void setName(std::string name); //Set Room Name
	std::string getName(); //Get Room Name
	std::vector<Client*> clients; //Clients in Room

private:
	std::string name;
	int index;
};

class ServerController {
public:

	ServerController(std::string IP, int PORT); //Constructer of Server Controller
	void CreateAccount(Client *&newClient); //Create Account For Connected Client
	bool CheckAccount(std::string id, std::string pw); //Check Account Registration with Client's ID and PW
	void AddRoom(std::string name); //Create New Chat Room
	void ShowRooms(int user_index); //List All Rooms
	void joinRoom(Client *&client, ChatRoom *&room); //Add Client to Selected Room
	void DisplayMenu(Client *&client); //Display Menu To Client
	void ServerStatus(); //List Server Info
	bool ListenForNewConnection(); //Listen IP Address for Connection
	void CloseConnection(SOCKET connection, Client *client); //Close Socket of Disconnected Client
	Client * Entrance(); //Show Entrance Operations to User

public:
	std::vector<ChatRoom*> rooms; //ChatRooms
	std::vector<Client *> allClients; //All Registred Clients
	std::vector<SOCKET> connections; //All Accepted Connections

private:
	bool SendInt(int index, int _int); //Send Size of Message to Client
	bool GetInt(int index, int &_int); //Receive Size of Message from Client
	bool SendString(int index, std::string &_string); //Send Message to Client 
	bool GetString(int index, std::string &_string); //Get Message from Client
	static void ClientHandlerThread(Client*& client); //Listen Client's for Message and Deploy Messages to Chat Rooms
	
private:
	std::unordered_map<std::string, std::string> accounts; //ID-PW 
	SOCKET listener; //Connection Socket Listener
	SOCKADDR_IN addr; //Address of Connection
	int addrlen = sizeof(addr);
	std::mutex thread_mutex;
};

static ServerController * serverptr;

#endif SERVER_AGENTS_