#include "Room.hh"

Room::Room(Websocket& server) : server(server)
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
	ss << connections.size() << ":" << maxPlayers;
	return ss;
}

void Room::handleMessage(Connection& cnn, std::string& cmd)
{
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
	else if(Util::subStr(cmd, 3) == "pos")
	{
		if(!positionVector(cmd, cnn))
		{
			std::cout << "Something went wrong!" << std::endl;
		}
		return;
	}
	else
	{
		std::cout << "Unknown message!" << std::endl;
	}
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

	float _x = Util::toFloat(x), _y = Util::toFloat(y), _z = Util::toFloat(x);

	if(!connections.find(cnn)->second.checkPos(_x, _y, _z))
		return false;

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





