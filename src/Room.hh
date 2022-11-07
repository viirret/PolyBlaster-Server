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
		void update();

		// react to messages sent in a room
		void handleMessage(Connection& cnn, const std::string& cmd);

		// Is it connected here?
		bool connectionHere(Connection& cnn) const;

		// add new client to room
		void addConnection(Connection& cnn, const std::string& playerID);

		Connection* leftRoom();

		std::ostringstream getStatus();
		ConnectionList getConnections();
		void leaveRoom(Connection& cnn);

	private:
		Connection* leftConnection = nullptr;

		bool updatePlayer(const std::string& cmd, const Connection& cnn);

		// send message to every client
		void broadcast(const std::string& cmd) const;

		// send message to every client, except yourself
		void broadcast(const std::string& cmd, const Connection& cnn) const;

		// all the connections in a room
		ConnectionList connections;

		// constructor variables:
		Websocket& server;
		std::string creator;
		int max;
		GameMode mode;
		bool friendlyFire = false;

		//int scoreA = 0, scoreB = 0, oldScoreA = 0, oldScoreB = 0, arg1 = 0;

		int scoreRed = 0, scoreBlue = 0, oldScoreRed = 0, oldScoreBlue = 0, arg1 = 0;

		// 0 for red, 1 for blue, -1 for undefined

		// storage for special commands
		enum class cmd
		{
			up,
			snd,
			dead,
			util,
			chat,
			myteam,
			getinfo,
			roominfo,
			leaveroom,
			newplayer,
			friendlyFire
		};

		// special commands
		std::map<std::string, cmd> commands = 
		{
			{"up", cmd::up},
			{"snd", cmd::snd},
			{"dead", cmd::dead},
			{"util", cmd::util},
			{"chat", cmd::chat},
			{"myteam", cmd::myteam},
			{"getinfo", cmd::getinfo},
			{"roominfo", cmd::roominfo},
			{"leaveroom", cmd::leaveroom},
			{"newplayer", cmd::newplayer},
			{"friendlyFire", cmd::friendlyFire}
		};
};

#endif
