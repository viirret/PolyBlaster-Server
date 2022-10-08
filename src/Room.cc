#include "Room.hh"

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

void Room::handleMessage(Connection& cnn, std::string& cmd)
{
	// STATIC MESSAGES

	// all the messages inside one room
	if(Util::subStr(cmd, 5) == "shoot")
	{
		for(auto& c : connections)
		{
			server.send(c.first, cmd, websocketpp::frame::opcode::text);
		}
		return;
	}
	else if(Util::subStr(cmd, 9) == "newplayer")
	{
		for(auto& c : connections)
		{
			server.send(c.first, cmd, websocketpp::frame::opcode::text);
		}
		return;
	}
	else if(Util::subStr(cmd, 4) == "util")
	{
		for(auto& c : connections)
		{
			server.send(c.first, cmd, websocketpp::frame::opcode::text);
		}
		return;
	}
	else if(Util::subStr(cmd, 3) == "rot")
	{
		for(auto& c : connections)
		{
			server.send(c.first, cmd, websocketpp::frame::opcode::text);
		}
		return;
	}
	else if(Util::subStr(cmd, 4) == "chat")
	{
		for(auto& c : connections)
		{
			server.send(c.first, cmd, websocketpp::frame::opcode::text);
		}
		return;
	}
	else if(Util::subStr(cmd, 4) == "arg1")
	{
		for(auto& c : connections)
		{
			server.send(c.first, "arg1:" + std::to_string(arg1), websocketpp::frame::opcode::text);
		}
		return;
	}
	if(Util::subStr(cmd, 12) == "friendlyFire")
	{
		int f = friendlyFire ? 1 : 0;

		for(auto& c : connections)
		{
			server.send(c.first, std::to_string(f), websocketpp::frame::opcode::text);
		}
		return;
	}

	else if(Util::subStr(cmd, 8) == "roominfo")
	{
		int x = (int)mode;
		for(auto& c : connections)
		{
			server.send(c.first, "roominfo:creator:" + creator + ":max:" + std::to_string(max) + ":mode:" + std::to_string(x), websocketpp::frame::opcode::text);
		}
		return;
	}
	
	else if(Util::subStr(cmd, 3) == "pos")
	{
		if(!positionVector(cmd, cnn))
		{
			std::cout << "Something went wrong!" << std::endl;
		}
		return;
	}

	// GAMEMODE SPESIFIC MESSAGES
	else if(mode == GameMode::team_deathmatch)
	{
		if(Util::subStr(cmd, 4) == "dead")
		{
			int n = 0;
			std::string command, player, team;
			for(std::string::size_type i = 0; i < cmd.size(); i++)
			{
				if(cmd[i] == ':')
				{
					n++;
				}
				else if(n <= 0)
				{
					command += cmd[i];
				}
				else if(n <= 1)
				{
					player += cmd[i];
				}
				else if(n <= 2)
				{
					team += cmd[i];
				}
			}

			// string to int
			int intteam = strTo<int>::value(team);

			// these might be opposite
			if(intteam == 0)
			{
				scoreA++;
			}
			else if(intteam == 1)
			{
				scoreB++;
			}

			for(auto& c : connections)
			{
				// here also respawn function
				server.send(c.first, cmd, websocketpp::frame::opcode::text);
			}
			return;
		}
	}

	else
	{
		std::cout << "Unknown message!" << std::endl;
	}
}

void Room::update()
{
	std::thread updateRoom([this]()
	{
		for(;;)
		{
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

			if(scoreA >= arg1)
			{
				for(auto& c : connections)
				{
					server.send(c.first, "victory:0", websocketpp::frame::opcode::text);
				}
				arg1 = 0;
			}

			if(scoreB >= arg1)
			{
				for(auto& c : connections)
				{
					server.send(c.first, "victory:1", websocketpp::frame::opcode::text);
				}
				arg1 = 0;
			}
		}
	});

	updateRoom.detach();
}

bool Room::positionVector(const std::string& cmd, const Connection& cnn)
{
	int n = 0;
	std::string arg, client, x, y, z;

	for(std::string::size_type i = 0; i < cmd.size(); i++)
	{
		if(cmd[i] == ':')
		{
			n++;
		}
		else
		{
			if(n <= 0)
			{
				arg += cmd[i];
			}
			else if(n <= 1)
			{
				client += cmd[i];
			}
			else if(n <= 2)
			{
				x += cmd[i];
			}
			else if(n <= 3)
			{
				y += cmd[i];
			}
			else if(n <= 4)
			{
				z += cmd[i];
			}
			else
			{
				std::cout << "Message failed!" << std::endl;
				return false;
			}
		}
	}

	std::string command;
	std::stringstream ss;

	ss << arg << ":" << client << ":" << x << ":" << y << ":" << z;
	ss >> command;

	// send position for each client
	for(auto& c : connections)
	{
		server.send(c.first, command, websocketpp::frame::opcode::text);
	}

	return true;
}





