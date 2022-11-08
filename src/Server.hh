#ifndef SERVER_HH
#define SERVER_HH

#include "Room.hh"
#include "Util.hh"

#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"

#include <string>
#include <set>
#include <unordered_map>
#include <map>

typedef Websocket::message_ptr Message;
typedef std::set<Connection, std::owner_less<Connection>> LobbyConnections;

class Server
{
	public:
		Server(int argc, char** argw);

	private:
		Room* findRoom(Connection& cnn);
		Player* findPlayer(Connection& cnn);
		void removeLobbyConnection(Connection cnn);
		void broadcast(const std::string& msg);

		Websocket server;
		LobbyConnections connections;

		int argc;
		char** argv;

		// default value for port
		unsigned port = 8080;

		std::unordered_map<std::string, Room> rooms;
		
		// connections from every room and lobby
		int allConnections = 0;

		// server commands
		enum class cmd
		{
			join,
			list, 
			create
		};

		std::map<std::string, cmd> commands =
		{
			{"join", cmd::join},
			{"list", cmd::list},
			{"create", cmd::create}
		};
};

#endif
