#ifndef SERVER_HH
#define SERVER_HH

#include "Room.hh"

#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"

#include <string>
#include <set>
#include <sstream>
#include <unordered_map>

typedef Websocket::message_ptr Message;
typedef std::set<Connection, std::owner_less<Connection>> LobbyConnections;

class Server
{
	public:
		Server(int argv, char** argc);
	private:
		Room* findRoom(Connection& cnn);
		void removeLobbyConnection(Connection cnn);

		Websocket server;
		LobbyConnections connections;

		int argv;
		char** argc;

		std::unordered_map<std::string, Room> rooms;
};

#endif
