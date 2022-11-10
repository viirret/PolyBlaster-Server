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
		Room(Websocket& server, std::string creator, int max, GameMode mode, bool friendlyFire, int scorethreshold, int warmup, int itemMap);
		void update();

		// react to messages sent in a room
		void handleMessage(Connection& cnn, const std::string& cmd);

		// Is it connected here?
		bool connectionHere(Connection& cnn) const;

		// add new client to room
		void addConnection(Connection& cnn, const std::string& playerID);

		// connection that has left the room
		Connection* leftRoom();

		// make connection leave room
		void leaveRoom(Connection& cnn);

		// get data from Room
		std::ostringstream getStatus();
		ConnectionList getConnections();

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
		bool friendlyFire;
		int scorethreshold;
		int warmup;
		int itemMap;
		
		bool isWarmup = true;
		size_t timeSinceCreation = 0;

		// 0 means red, 1 means blue; clientside numbering
		int scoreRed = 0, scoreBlue = 0, oldScoreRed = 0, oldScoreBlue = 0;

		// commands for a room
		enum class cmd
		{
			up,
			snd,
			map,
			dead,
			item,
			util,
			chat,
			myteam,
			getinfo,
			roominfo,
			leaveroom,
			newplayer,
			getplayers,
		};

		std::map<std::string, cmd> commands = 
		{
			{"up", cmd::up},
			{"snd", cmd::snd},
			{"map", cmd::map},
			{"dead", cmd::dead},
			{"item", cmd::item},
			{"util", cmd::util},
			{"chat", cmd::chat},
			{"myteam", cmd::myteam},
			{"getinfo", cmd::getinfo},
			{"roominfo", cmd::roominfo},
			{"leaveroom", cmd::leaveroom},
			{"newplayer", cmd::newplayer},
			{"getplayers", cmd::getplayers}
		};
};

#endif
