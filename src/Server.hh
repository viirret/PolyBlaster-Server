#ifndef SERVER_HH
#define SERVER_HH

#include "Room.hh"

#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"

#include <string>
#include <set>
#include <sstream>
#include <unordered_map>

typedef websocketpp::server<websocketpp::config::asio> Websocket;
typedef websocketpp::connection_hdl Connection;
typedef Websocket::message_ptr Message;

class Server
{
	public:
		Server(int argv, char** argc);
	private:
		Room* findRoom(Connection& cnn);

		Websocket server;

		int argv;
		char** argc;

		std::unordered_map<std::string, Room> rooms;
};

#endif
