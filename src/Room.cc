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

void Room::handleMessage(Connection& cnn, const std::string& msg)
{
	switch(commands[msg])
	{
		case cmd::pos:
		{
			positionVector(msg, cnn);
			return;
		}

		// NOTE: this only works for the team deathmatch mode
		case cmd::dead:
		{
			int n = 0;
			std::string command, player, team;
			for(std::string::size_type i = 0; i < msg.size(); i++)
			{
				if(msg[i] == ':')
				{
					n++;
				}
				else if(n <= 0)
				{
					command += msg[i];
				}
				else if(n <= 1)
				{
					player += msg[i];
				}
				else if(n <= 2)
				{
					team += msg[i];
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

			broadcast(msg);
			return;
		}
		
		case cmd::friendlyFire:
		{
			broadcast("friendlyFire:" + std::to_string(friendlyFire));
			return;
		}
		case cmd::roominfo:
		{
			broadcast("roominfo:" + creator + ":" + std::to_string(max) + ":" 
			+ std::to_string(connections.size()) + ":" + std::to_string(arg1));
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

/*
void Room::excludeOwn(const std::string& msg, const Connection& cnn)
{
	for(auto& c : connections)
	{
		// compare c.first and cnn
		if(c.first != cnn)
			server.send(c.first, msg, websocketpp::frame::opcode::text);
	}
}
*/

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





