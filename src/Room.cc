#include "Room.hh"

#include <memory>

// differentiate std::weak ptr from each other
template <typename T, typename U>
inline bool equals(const std::weak_ptr<T>& c1, const std::weak_ptr<U>& c2)
{
	return !c1.owner_before(c2) && !c2.owner_before(c1);
}

Room::Room(Websocket& server, std::string creator, int max, GameMode mode, bool friendlyFire, int arg1)
	: server(server), creator(creator), max(max), mode(mode), friendlyFire(friendlyFire), arg1(arg1)
{
}

bool Room::connectionHere(Connection& cnn)
{
	return connections.find(cnn) != connections.end();
}

ConnectionList Room::getConnections() { return connections; }

void Room::addConnection(Connection& cnn, const std::string& playerID)
{
	connections.emplace(cnn, Player(playerID));

	auto conn = server.get_con_from_hdl(cnn);
	conn->set_close_handler([this](Connection connection)
	{
		connections.erase(connections.find(connection));
	});

	// inform the client that it joined the game
	server.send(cnn, "joined", websocketpp::frame::opcode::text);
}

std::ostringstream Room::getStatus()
{
	std::ostringstream ss;
	ss << connections.size() << ":" << max;
	return ss;
}

void Room::handleMessage(Connection& cnn, const std::string& msg)
{
	// identifier for command, works for all commands
	std::string fw;
	for(std::string::size_type i = 0; i < msg.size(); i++)
	{
		if(msg[i] == ':') break;
		else fw += msg[i];
	}

	switch(commands[fw])
	{
		case cmd::up:
		{
			if(!updatePlayer(msg, cnn))
			{
				std::cout << "Updating player failed!" << std::endl;
			}
			return;
		}

		case cmd::dead:
		{
			// normal death
			int n = 0;
			std::string player, team;
			for(std::string::size_type i = 0; i < msg.size(); i++)
			{
				if(msg[i] == ':')
					n++;
				else
				{
					switch(n)
					{
						case 0: break;
						case 1: player += msg[i]; break;
						case 2: team += msg[i]; break;
					}
				}
			}

			// send message back to client confirming its death
			server.send(cnn, msg, websocketpp::frame::opcode::text);

			if(mode == GameMode::team_deathmatch)
			{
				// string to int
				int intteam = strTo<int>::value(team);

				// update scores
				if(intteam == 0)
					scoreA++;
				else if(intteam == 1)
					scoreB++;
				
				// send message back to server about respawning
				server.send(cnn, "respawn", websocketpp::frame::opcode::text);

				return;
			}
			return;
		}

		case cmd::myteam:
		{
			int n = 0;
			std::string player, team;
			for(std::string::size_type i = 0; i < msg.size(); i++)
			{
				if(msg[i] == ':')
					n++;
				else
				{
					switch(n)
					{
						case 0: break;	
						case 1: player += msg[i]; break;
						case 2: team += msg[i]; break;
						default: std::cout << "Something went wrong!" << std::endl; break;
					}
				}

				// set the team for player
				for(auto& c : connections)
					if(c.second.getId() == player)	
						c.second.setTeam(strTo<int>::value(team));

			}
			return;
		}

		case cmd::getinfo:
		{
			std::string info = "getinfo:";

			for(auto& c : connections)
			{
				// do not send information about yourself, to yourself
				if(!equals(c.first, cnn))
				{
					info += c.second.getId();
					info += ":";
					info += c.second.getPos();
					info += ":";
					info += std::to_string(c.second.getTeam());
					info += ";";
				}
			}
			
			server.send(cnn, info, websocketpp::frame::opcode::text);
			return;
		}
			
		case cmd::friendlyFire:
		{
			server.send(cnn, "friendlyFire:" + std::to_string(friendlyFire), websocketpp::frame::opcode::text);
			return;
		}
		case cmd::roominfo:
		{
			server.send(cnn, "roominfo:" + creator + ":" + std::to_string(max) + ":" 
			+ std::to_string(connections.size()) + ":" + std::to_string(arg1), websocketpp::frame::opcode::text);
			return;
		}
		
		// by default all messages are sent to every client,
		// these messages are not to be sent to itself
		case cmd::newplayer:
		{
			broadcast(msg, cnn);
			return;
		}
	}

	// TODO find out all commands and you client needs to send it itself
	// and run them in their own loop, or same why not
	broadcast(msg);
}

void Room::broadcast(const std::string& msg)
{
	for(auto& c : connections)
		server.send(c.first, msg, websocketpp::frame::opcode::text);
}

void Room::broadcast(const std::string& msg, const Connection& cnn)
{
	for(auto& c : connections)
		if(!equals(c.first, cnn))
			server.send(c.first, msg, websocketpp::frame::opcode::text);
}

void Room::update()
{
	std::thread updateRoom([this]()
	{
		for(;;)
		{
			if(scoreA >= arg1)
			{
				for(auto& c : connections)
				{
					server.send(c.first, "victory:1", websocketpp::frame::opcode::text);
				}
				scoreA = 0;
				scoreB = 0;
			}

			if(scoreB >= arg1)
			{
				for(auto& c : connections)
				{
					server.send(c.first, "victory:0", websocketpp::frame::opcode::text);
				}
				scoreB = 0;
				scoreA = 0;
			}

			if(oldScoreA != scoreA || oldScoreB != scoreB)
			{
				std::string updateScore = "updatescores:" + std::to_string(scoreA) + ":" + std::to_string(scoreB);

				for(auto& c : connections)
				{
					server.send(c.first, updateScore, websocketpp::frame::opcode::text);
				}

				oldScoreA = scoreA;
				oldScoreB = scoreB;
			}
		}
	});

	updateRoom.detach();
}

bool Room::updatePlayer(const std::string& cmd, const Connection& cnn)
{
	int n = 0;
	std::string arg, client, px, py, pz, rx, ry, rz;

	for(std::string::size_type i = 0; i < cmd.size(); i++)
	{
		if(cmd[i] == ':')
			n++;
		else
		{
			switch(n)
			{
				case 0: arg += cmd[i]; break;
				case 1: client += cmd[i]; break;
				case 2: px += cmd[i]; break;
				case 3: py += cmd[i]; break;
				case 4: pz += cmd[i]; break;
				case 5: rx += cmd[i]; break;
				case 6: ry += cmd[i]; break;
				case 7: rz += cmd[i]; break;
				default: 
				{
					std::cout << "Message failed!" << std::endl;
					return false;
				}
			}
		}
	}

	std::string command;
	std::stringstream ss;

	ss << arg << ":" << client << ":" << px << ":" << py << ":" << pz << ":" << rx << ":" << ry << ":" << rz;
	ss >> command;

	float x = strTo<float>::value(px);
	float y = strTo<float>::value(py);
	float z = strTo<float>::value(pz);

	// update position of the player
	for(auto& c : connections)
		if(c.second.getId() == client)
			c.second.changePos(x, y, z);


	// here we can do some checks for the command, if needed in the future

	// send position for each client except yourself
	broadcast(command, cnn);

	return true;
}





