#ifndef ROOM_HH
#define ROOM_HH

#include "Util.hh"
#include "Player.hh"
#include "GameItem.hh"

#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"

#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <map>

typedef websocketpp::server<websocketpp::config::asio> Websocket;
typedef websocketpp::connection_hdl Connection;
typedef std::map<Connection, Player, std::owner_less<Connection>> ConnectionList;

enum class GameMode
{
	team_deathmatch,
	collect_items,
	hardpoint,
	tdm_with_static_spawns
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
		void addConnection(Connection& cnn, const std::string& playerID, const std::string& username);

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

		// add information about the map to the server
		bool createMap(const std::string& cmd, const Connection& cnn);

		// command that has GameItems in the message
		std::string gameItemCommand(const std::string& cmd, std::vector<GameItem>& gItem);

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
		size_t warmupTime = 0;

		// 0 means red, 1 means blue; clientside numbering
		int scoreRed = 0, scoreBlue = 0, oldScoreRed = 0, oldScoreBlue = 0;

		// serverside items in a Room
		std::vector<GameItem> items;

		// currently picked up items
		std::vector<std::pair<GameItem, int>> deletedItems;
		int recoverTime = 10;
		
		std::vector<GameItem> zones;

		// hazard zone (hardpoint)
		
		// the actual timer
		int hazardTimer = 0;

		// hazard zone lenght
		const int hazardZoneTime = 60;

		// cooldown time
		const int hazardZoneCooldown = 30;
		
		bool hazardZoneOn = true;

		// currently playing hazardzone as int
		int currentHazardIndex = 0;

		// scoring with hardpoints
		const int originalHardPointScore = 20;
		int hardpointScore = 20;

		// commands for a room
		enum class cmd
		{
			up,
			snd,
			map,
			asnd,
			dead,
			util,
			item,
			zone,
			chat,
			myteam,
			reload,
			getinfo,
			getzone,
			getscore,
			roominfo,
			hardpoint,
			leaveroom,
			newplayer,
		};

		std::map<std::string, cmd> commands
		{
			{"up", cmd::up},
			{"snd", cmd::snd},
			{"map", cmd::map},
			{"asnd", cmd::asnd},
			{"dead", cmd::dead},
			{"util", cmd::util},
			{"zone", cmd::zone},
			{"item", cmd::item},
			{"chat", cmd::chat},
			{"myteam", cmd::myteam},
			{"reload", cmd::reload},
			{"getinfo", cmd::getinfo},
			{"getzone", cmd::getzone},
			{"getscore", cmd::getscore},
			{"roominfo", cmd::roominfo},
			{"hardpoint", cmd::hardpoint},
			{"leaveroom", cmd::leaveroom},
			{"newplayer", cmd::newplayer},
		};

		// I am gradually trying to increase these until I have all of them
		std::vector<std::string> protectedCommands
		{
			"up", "snd", "asnd", "dead", "util", "zone", "chat", "myteam",
			"hardpoint", "leaveroom", "newplayer"
		};

		// TODO
		// MAP, ITEM, GETZONE
};

#endif
