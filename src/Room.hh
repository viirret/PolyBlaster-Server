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

enum class GameMode
{
	team_deathmatch,
	something_else
};

class Room
{
	public:
		Room(Websocket& server, std::string creator, int max, GameMode mode, bool friendlyFire, int arg1);

		void handleMessage(Connection& cnn, const std::string& cmd);
		bool connectionHere(Connection& cnn);
		void addConnection(Connection& cnn, const std::string& playerID);
		void update();

		std::ostringstream getStatus();
		ConnectionList getConnections();

	private:
		bool updatePlayer(const std::string& cmd, const Connection& cnn);

		// send message to every client
		void broadcast(const std::string& cmd);

		// send message to every client, except yourself
		void broadcast(const std::string& cmd, const Connection& cnn);

		ConnectionList connections;
		Websocket& server;

		std::string creator;

		// default value for max players
		int max = 10;
		GameMode mode;

		int scoreA = 0, scoreB = 0, oldScoreA = 0, oldScoreB = 0, arg1 = 0;
		bool friendlyFire = false;

		// storage for special commands
		enum class cmd
		{
			up,
			dead,
			roominfo,
			friendlyFire,
		};

		// special commands
		std::map<std::string, cmd> commands = 
		{
			{"up", cmd::up},
			{"dead", cmd::dead},
			{"roominfo", cmd::roominfo},
			{"friendlyFire", cmd::friendlyFire}
		};
};

#endif
