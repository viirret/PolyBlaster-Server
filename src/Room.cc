#include "Room.hh"

Room::Room(Websocket& server) : server(server)
{
}

bool Room::connectionHere(Connection& cnn)
{
	return connections.find(cnn) != connections.end();
}

void Room::addConnection(Connection& cnn)
{
	connections.emplace(cnn);
}

std::ostringstream Room::getStatus()
{
	std::ostringstream ss;
	ss << connections.size() << ":" << maxPlayers;
	return ss;
}

void Room::handleMessage(Connection& cnn, std::string& cmd)
{
	if(Util::subStr(cmd, 5) == "shoot")
	{
		for(auto& c : connections)
		{
			server.send(c, cmd, websocketpp::frame::opcode::text);
		}
		return;
	}
	
	else if(Util::subStr(cmd, 3) == "pos")
	{
		if(!positionVector(cmd))
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

bool Room::positionVector(const std::string& cmd)
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
		server.send(c, command, websocketpp::frame::opcode::text);
	}

	return true;
}





