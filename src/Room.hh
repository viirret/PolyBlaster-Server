#ifndef ROOM_HH
#define ROOM_HH

#include "Util.hh"

#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"

#include <set>
#include <sstream>
#include <string>

typedef websocketpp::server<websocketpp::config::asio> Websocket;
typedef websocketpp::connection_hdl Connection;
typedef std::set<Connection, std::owner_less<Connection>> ConnectionList;

class Room
{
	public:
		Room(Websocket& server);

		void handleMessage(Connection& cnn, std::string& cmd);
		bool connectionHere(Connection& cnn);
		void addConnection(Connection& cnn);

		std::ostringstream getStatus();
	
	private:
		bool positionVector(const std::string& cmd);

		ConnectionList connections;
		Websocket& server;

		int maxPlayers = 10;
};

#endif
