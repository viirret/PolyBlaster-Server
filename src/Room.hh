#ifndef ROOM_HH
#define ROOM_HH

#include "Util.hh"
#include "Player.hh"

#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"

#include <set>
#include <sstream>
#include <string>
#include <map>

typedef websocketpp::server<websocketpp::config::asio> Websocket;
typedef websocketpp::connection_hdl Connection;
typedef std::map<Connection, Player, std::owner_less<Connection>> ConnectionList;

class Room
{
	public:
		Room(Websocket& server);

		void handleMessage(Connection& cnn, std::string& cmd);
		bool connectionHere(Connection& cnn);
		void addConnection(Connection& cnn, const std::string& playerID);

		std::ostringstream getStatus();

	private:
		bool positionVector(const std::string& cmd, const Connection& cnn);

		ConnectionList connections;
		Websocket& server;

		// default value for max players
		int maxPlayers = 10;
};

#endif
