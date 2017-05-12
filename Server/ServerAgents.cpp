#include "ServerAgents.h"
#include "utils.h"

Client::Client() {

}

//Creating Copy of Current Client
Client* Client::copy() {

	Client* clientCopy = new Client();

	clientCopy->curr_room = this->curr_room;
	clientCopy->glob_index = this->glob_index;
	clientCopy->room_index = this->room_index;
	clientCopy->name = this->name;
	clientCopy->id = this->id;
	clientCopy->pw = this->pw;
	clientCopy->connection_index = this->connection_index;

	return clientCopy;
}

ChatRoom::ChatRoom() {

}

ChatRoom::ChatRoom(std::string name) {
	this->name = name;
}

std::string ChatRoom::getName() {
	return this->name;
}

void ChatRoom::setName(std::string name) {
	this->name = name;
}

int ChatRoom::getIndex() {
	return this->index;
}

void ChatRoom::setIndex(int index) {
	this->index = index;
}

ServerController::ServerController(std::string IP, int PORT) {

	//WSOCK SETUP
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);

	//if WSAStartup return anything other than 0, then it is an error
	if (WSAStartup(DllVersion, &wsaData) != 0) {
		MessageBox(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}


	//Server Config
	SOCKADDR_IN addr; //Address that we will bind our listening socket to
	int addr_len = sizeof(addr); //length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr(IP.c_str()); //Broadcast Locally
	addr.sin_port = htons(PORT); //Port
	addr.sin_family = AF_INET; //IPv4 Socket

	//Socket Config
	this->listener = socket(AF_INET, SOCK_STREAM, NULL); //Create socket to listen for new connections

	if (bind(listener, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) { //Bind the address to the socket
		std::string msg = "Failed to bind the address to our listening socket. Winsock Error : " + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, msg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	if (listen(listener, SOMAXCONN) == SOCKET_ERROR) { //Places listener socket in a state in which it is listening for an incoming connection
														//Note:SOMAXCONN = Socket Outstanding Max Connection
		std::string ErrorMsg = "Failed to listen on listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	serverptr = this; //Initializing the serverptr with this for the static function access other functions
}

//Method for Send Integer to Users (@general usage : size of string to send)
bool ServerController::SendInt(int index, int _int) {
	if (send(this->connections[index], (char*)&_int, sizeof(int), NULL) == SOCKET_ERROR) // if int failed to send due to connection issue
		return false;
	return true;
}

//Method for Receive Integers from Users
bool ServerController::GetInt(int index, int &_int) {
	if (recv(this->connections[index], (char*)&_int, sizeof(int), NULL) == SOCKET_ERROR)
		return false;
	return true;
}

//Method for Send Message to Users
bool ServerController::SendString(int index, std::string &_string) {
	int buffer_size = _string.size();

	if (!this->SendInt(index, buffer_size)) //Send Size of Message
		return false;

	if (send(this->connections[index], _string.c_str(), buffer_size, NULL) == SOCKET_ERROR) //Send Message to User
		return false;

	return true;
}

//Receive String From User
bool ServerController::GetString(int index, std::string &_string) {
	int buffer_size;
	if (!this->GetInt(index, buffer_size)) //Get Size of Message
		return false;

	char * buffer = new char[buffer_size + 1]; //Allocate Memory for Message

	buffer[buffer_size] = '\0';

	if (recv(this->connections[index], buffer, buffer_size, NULL) == SOCKET_ERROR) //Receive Message From User
		return false;

	_string = buffer;
	delete[] buffer;//Delete Allocated Memory

	return true;
}

//Method For Account Registration
void ServerController::CreateAccount(Client* &newClient) {
	this->accounts[newClient->id] = newClient->pw; //Save ID and PW Information Of User For Login
	this->allClients.push_back(std::move(newClient->copy()));  //Save Full Information for Global Operations
}

//Check Users Registration
bool ServerController::CheckAccount(std::string id, std::string pw) {
	return this->accounts[id].compare(pw) == 0;
}

//Method for Creating Chat Room
void ServerController::AddRoom(std::string name) {
	ChatRoom* newRoom = new ChatRoom();
	newRoom->setName(name);
	newRoom->setIndex(this->rooms.size());
	this->rooms.push_back(std::move(newRoom));
}

//List Rooms to User's Interface
void ServerController::ShowRooms(int user_index) {
	for (unsigned int i = 0; i < this->rooms.size(); i++) {
		std::string room = "Room Index : " + std::to_string(i + 1) + "| Room Name : " + this->rooms[i]->getName() + 
						   "| Capacity : " + std::to_string(this->rooms[i]->clients.size()) + "/" + std::to_string(MAX_CAPACITY);
		this->SendString(user_index, room);
	}
}

//Log Server Status
void ServerController::ServerStatus() {
	system("cls");
	std::cout << "SERVER STATUS" << std::endl;
	std::cout << "ROOMS" << std::endl;
	for (unsigned int i = 0; i < this->rooms.size(); i++) {
		std::string room_info = "Room Index : " + std::to_string(i + 1) + 
								"| Room Name : " + this->rooms[i]->getName() +
								"| Capacity : " + std::to_string(this->rooms[i]->clients.size()) + "/" + std::to_string(MAX_CAPACITY);
		std::cout << room_info << std::endl;
	}

	std::cout << "ACTIVE USERS" << std::endl;
	for (unsigned int i = 0; i < this->allClients.size(); i++) {
		if (this->allClients[i]->isInRoom) {
			std::string user_info = "ID : " + std::to_string(this->allClients[i]->glob_index) +
				"| Username : " + this->allClients[i]->name +
				"| Room : " + this->rooms[this->allClients[i]->curr_room]->getName();
			std::cout << user_info << std::endl;
		}
	}
}

//Insert User to Selected Chat Room
void ServerController::joinRoom(Client* &client, ChatRoom* &room) {
	client->room_index = room->clients.size();
	client->curr_room = room->getIndex();
	room->clients.push_back(std::move(client->copy()));
}

//Entrance Operations
Client* ServerController::Entrance() {

	std::string entrance = "1 - Login\n2 - Create Account";
	std::string choice;
	
	Client* client = new Client();

	client->connection_index = this->connections.size() - 1;

	this->SendString(client->connection_index, entrance);
	
	this->GetString(client->connection_index, choice);

	if (choice.compare("1") == 0) {

		std::string id = "id : ";
		this->SendString(client->connection_index, id);
		this->GetString(client->connection_index, client->id);

		std::string pw = "pw : ";
		this->SendString(client->connection_index, pw);
		this->GetString(client->connection_index, client->pw);

		if (!this->CheckAccount(client->id, client->pw)) {

			std::string name = "name : ";

			this->SendString(client->connection_index, name);
			this->GetString(client->connection_index, client->name);

			client->glob_index = this->allClients.size();
			
			this->CreateAccount(client);
			SEND_CL_MSG(this->SendString, client->connection_index);
		}
		else {
			std::string login_msg = "Login Succesfull";
			for (auto& i : this->allClients) {
				if (i->id == client->id && i->pw == client->pw) {
					client->glob_index = i->glob_index;
					this->allClients[i->glob_index]->connection_index = client->connection_index;
					SEND_CL_MSG(this->SendString, client->connection_index);
					this->SendString(client->connection_index, login_msg);
					return this->allClients[i->glob_index]->copy();
				}
			}
		}
	}
	else if (choice.compare("2") == 0) {
	
		std::string name = "name : ";
		this->SendString(client->connection_index, name);
		this->GetString(client->connection_index, client->name);

		std::string id = "id : ";
		this->SendString(client->connection_index, id);
		this->GetString(client->connection_index, client->id);

		std::string pw = "pw : ";
		this->SendString(client->connection_index, pw);
		this->GetString(client->connection_index, client->pw);

		client->glob_index = this->allClients.size();

		client->connection_index = client->connection_index;

		this->CreateAccount(client);

		SEND_CL_MSG(this->SendString, client->connection_index);
	}

	return client;
}


void ServerController::DisplayMenu(Client*& client) {
	int user_index = client->connection_index;
	int index = client->glob_index;

	while (true) {
		
		std::string entrance = "1 - Show Chat Rooms\n2 - Join A Chat Room\n";
		std::string choice;

		this->SendString(user_index, entrance);
		this->GetString(user_index, choice);
		SEND_CL_MSG(this->SendString, user_index);

		if (choice.compare("1") == 0) {
			this->ShowRooms(user_index);
		}
		else if (choice.compare("2") == 0) {
			std::string room_selection = "Select A Room";
			std::string r_choice;

			this->SendString(user_index, room_selection);
			this->ShowRooms(user_index);
			this->GetString(user_index, r_choice);

			int room_index = std::atoi(r_choice.c_str());

			this->joinRoom(this->allClients[index], this->rooms[room_index]);

			client->curr_room = this->allClients[index]->curr_room;
			client->room_index = this->allClients[index]->room_index;
			client->isInRoom = true;

			break;
		}
	}
}

bool ServerController::ListenForNewConnection() {
	SOCKET newConnection = accept(this->listener, (SOCKADDR*)&addr, &this->addrlen); //Accept a new connection
	if (newConnection == 0) //If accepting the client connection failed
	{
		std::cout << "Failed to accept the client's connection." << std::endl;
		return false;
	}
	else //If client connection properly accepted
	{

		std::cout << "LOG MESSAGE : Client Connected!" << std::endl;
		this->connections.push_back(newConnection); //Set socket in array to be the newest connection before creating the thread to handle this client's socket.
		
		Client* newClient = new Client();

		SEND_CL_MSG(this->SendString, this->connections.size() - 1);//Send Clear Message to Client for Clear His/Her Interface

		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)(newClient/*serverptr->allClients[newClient->glob_index]*/), NULL, NULL); //Create Thread to handle this client. The index in the socket array for this thread is the value (i).

		return true;
	}
}

//Close connection if Client Disconnect From Chat Room
void ServerController::CloseConnection(SOCKET connection, Client* client) {
	closesocket(connection);//Close Connection Socket
	
	int deleted_room_index = client->room_index; //Index of disconnected Client
	
	//Left Shift Other Users's Room Indexes
	for (unsigned int i = deleted_room_index + 1; i < serverptr->rooms[client->curr_room]->clients.size(); i++) {
		serverptr->rooms[client->curr_room]->clients[i]->room_index--; 
		serverptr->allClients[serverptr->rooms[client->curr_room]->clients[i]->glob_index]->room_index--;
	}

	serverptr->rooms[client->curr_room]->clients.erase(serverptr->rooms[client->curr_room]->clients.begin() + client->room_index); //Delete User Info From Room
}


void ServerController::ClientHandlerThread(Client*& client) {

	client = std::move(serverptr->Entrance());

	serverptr->DisplayMenu(client);//Display Menu To Connected Client

	serverptr->ServerStatus();//Show Status After Every New Connection

	serverptr->SendString(client->connection_index, START_CHAT_MSG(client->name, serverptr->rooms[client->curr_room]->getName()));

	int user_index = client->connection_index;

	int glob_index = client->glob_index;

	while (client->isInRoom) {

		std::string msg;

		if (!serverptr->GetString(user_index, msg)) { //Get Message From User
			break;
		}

		msg = serverptr->allClients[glob_index]->name + " : " + msg;

		//Send Message Clients only which are in same room with Sender
		for (auto& room_mate : serverptr->rooms[client->curr_room]->clients) {
			if (room_mate->connection_index == user_index) //Don't Send Message to Sender
				continue;

			if (!serverptr->SendString(room_mate->connection_index, msg)) { //Send Message To Room 
				std::cout << "Error happened while sending message from client : " << user_index << "to client :" << room_mate->glob_index << std::endl;
			}
		}
		std::cout << "LOG MESSAGE : Message sent succesfully from user : " << user_index << std::endl;
	}

	std::cout << "LOG MESSAGE : Client Disconnected: " << user_index << std::endl;

	serverptr->CloseConnection(serverptr->connections[user_index], serverptr->allClients[glob_index]->copy());	
}

