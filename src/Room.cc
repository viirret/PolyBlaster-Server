#include "Room.hh"

Room::Room(Websocket& server, std::string creator, int max, GameMode mode, bool friendlyFire, int scorethreshold, int warmup, int itemMap)
	: server(server), creator(creator), max(max), mode(mode), friendlyFire(friendlyFire), scorethreshold(scorethreshold), warmup(warmup), itemMap(itemMap)
{
}

bool Room::connectionHere(Connection& cnn) const
{
	return connections.find(cnn) != connections.end();
}

ConnectionList Room::getConnections() { return connections; }

void Room::addConnection(Connection& cnn, const std::string& playerID)
{
	connections.emplace(cnn, Player(playerID));

	// inform the client of joining the game
	server.send(cnn, "joined", websocketpp::frame::opcode::text);

	// get information about the map in the Room
	server.send(cnn, "map:" + std::to_string(itemMap), websocketpp::frame::opcode::text);
}

Connection* Room::leftRoom()
{
	Connection* c = leftConnection;
	leftConnection = nullptr;
	return c;
}

void Room::leaveRoom(Connection& cnn)
{
	// get id from player who is leaving
	auto id = connections.find(cnn)->second.getId();

	// inform other players of leaving the room
	for(auto& c : getConnections())
		if(!Util::equals(c.first, cnn))
			server.send(c.first, "quit:" + id, websocketpp::frame::opcode::text);
	
	// remove from room's connections
	connections.erase(connections.find(cnn));
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

		case cmd::leaveroom:
		{
			leftConnection = &cnn;
			leaveRoom(cnn);
			break;
		}

		case cmd::zone:
		{
			if(zones.empty())
			{
				server.send(cnn, "getzone", websocketpp::frame::opcode::text);
			}
			return;
		}

		case cmd::getzone:
		{
			// set zones as serverside items
			gameItemCommand(msg, zones);

			// print created zones
			for(auto& z : zones)
			{
				std::cout << "ID " << z.id << " X " << z.x << " Y " << z.y << " Z " << z.z << std::endl;
			}

			return;
		}

		case cmd::item:
		{
			int n = 0;
			std::string id, x, y, z, tag, client, team;

			// parse message
			for(std::string::size_type i = 0; i < msg.size(); i++)
			{
				if(msg[i] == ':')
					n++;
				else if(msg[i] == ';')
					continue;
				else
				{
					switch(n)
					{
						case 1: id += msg[i]; break;
						case 2: x += msg[i]; break;
						case 3: y += msg[i]; break;
						case 4: z += msg[i]; break;
						case 5: tag += msg[i]; break;
						case 6: client += msg[i]; break;
						case 7: team += msg[i]; break;
					}
				}
			}
			
			// printing message fields
			std::cout << "ID " << id << " x " << x << " y " << y << " z " << z << " tag " << tag << " client " << client << " team " << team << std::endl;

			// do stuff based in ServerItem.tag (add score, add health, etc)
			
			if(tag == "pointItem")
			{
				if(team == "0")
				{
					scoreRed++;
				}
				else if(team == "1")
				{
					scoreBlue++;
				}
			}
			else if(tag == "healthBox")
			{
				server.send(cnn, "healthBox", websocketpp::frame::opcode::text);
			}
			else if(tag == "ammoBox")
			{
				server.send(cnn, "ammoBox", websocketpp::frame::opcode::text);
			}

			// destroy item from server

			// destroy item in clients
			
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
				int intteam = strTo<int>::value(team);

				// update scores
				if(intteam == 0)
					scoreRed++;
				else if(intteam == 1)
					scoreBlue++;
				
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

		// get information about every player in room
		case cmd::getinfo:
		{
			std::string info = "getinfo:";

			for(auto& c : connections)
			{
				// do not send information about yourself, to yourself
				if(!Util::equals(c.first, cnn))
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

		case cmd::map:
		{
			if(!createMap(msg, cnn))
			{
				std::cout << "Couldn't pass map commands!" << std::endl;
			}
			return;
		}


		// get all information about the room
		case cmd::roominfo:
		{
			server.send(cnn, "roominfo:" + creator + ":" + std::to_string(max) + ":" + std::to_string(connections.size()) + 
			":" + std::to_string(friendlyFire) + ":" + std::to_string(scorethreshold) + ":" + std::to_string(warmup) + ":" + 
			std::to_string(itemMap) + ":" + std::to_string(scoreRed) + ":" + std::to_string(scoreBlue), 

			websocketpp::frame::opcode::text);
			return;
		}

		// get identifiers from all players in a room		
		case cmd::getplayers:
		{
			std::string info = "getplayers:";

			for(auto& c : connections)
				info += c.second.getId() + ":";
			
			broadcast(info);
			return;
		}
		
		// these messages are not to be sent to itself
		case cmd::newplayer: case cmd::snd:
		{
			broadcast(msg, cnn);
			return;
		}

		// these messages are to be sent to every client
		case cmd::util: case cmd::chat:
		{
			broadcast(msg);
			return;
		}

		// inform client of faulty command
		server.send(cnn, "invalid", websocketpp::frame::opcode::text);
	}
}

void Room::broadcast(const std::string& msg) const
{
	for(auto& c : connections)
		server.send(c.first, msg, websocketpp::frame::opcode::text);
}

void Room::broadcast(const std::string& msg, const Connection& cnn) const
{
	for(auto& c : connections)
		if(!Util::equals(c.first, cnn))
			server.send(c.first, msg, websocketpp::frame::opcode::text);
}

void Room::update()
{
	std::thread updateRoom([this]()
	{
		for(;;)
		{
			// update scores	
			if(scoreRed >= scorethreshold)
			{
				broadcast("victory:0");

				scoreRed = 0;
				scoreBlue = 0;
			}

			if(scoreBlue >= scorethreshold)
			{
				broadcast("victory:1");

				scoreRed = 0;
				scoreBlue = 0;
			}

			if(oldScoreRed != scoreRed || oldScoreBlue != scoreBlue)
			{
				std::string updateScore = "updatescores:" + std::to_string(scoreRed) + ":" + std::to_string(scoreBlue);

				broadcast(updateScore);

				oldScoreRed = scoreRed;
				oldScoreBlue = scoreBlue;
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));

		}
	});

	std::thread updateWarmup([this]()
	{
		for(;;)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			timeSinceCreation++;

			if(timeSinceCreation <= (size_t)warmup)
			{
				// send message about warmup time
				broadcast("warmup:" + std::to_string(timeSinceCreation) + ":" + std::to_string(warmup));
			}
		}
	});

	std::thread updateHazardZone([this]()
	{
		for(;;)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));

			// hazardZones have not been assigned
			if(zones.empty())
				continue;

			// hazardtimer or cooldowntimer hasn't been reached
			if((hazardTimer < hazardZoneTime && hazardZoneOn) || (hazardTimer < hazardZoneCooldown && !hazardZoneOn))
			{
				hazardTimer++;	
			}

			// hazardtimer has been reached
			else if(hazardTimer >= hazardZoneTime && hazardZoneOn)
			{
				hazardZoneOn = false;
				hazardTimer = 0;
			}
			
			// cooldown has been reached
			else if(hazardTimer >= hazardZoneCooldown && !hazardZoneOn)
			{
				hazardZoneOn = true;
				hazardTimer = 0;
				currentHazardIndex = Util::randomValue(0, zones.size());
			}

			std::string str = "hazard:";
			str += hazardZoneOn ? "1:" : "0:";
			str += zones[currentHazardIndex].print();

			broadcast(str);
		}
	});

	updateRoom.detach();
	updateWarmup.detach();
	updateHazardZone.detach();
}

bool Room::createMap(const std::string& cmd, const Connection& cnn)
{
	std::cout << "MAP COMMAND" << std::endl;
	std::cout << cmd << std::endl;

	std::string map = gameItemCommand(cmd, items);

	std::string mapType;
	for(std::string::size_type i = 0; i < map.size(); i++)
		if(i == 4)
			mapType = map[i];

	if(itemMap == strTo<int>::value(mapType))
	{
		// tell client to create the map
		server.send(cnn, "map:" + mapType, websocketpp::frame::opcode::text);
		return true;
	}
	else
		return false;
}

std::string Room::gameItemCommand(const std::string& cmd, std::vector<GameItem>& gItem)
{
	int j = 0, k = 0;
	std::string body, id, x, y, z, tag;
	for(std::string::size_type i = 0; i < cmd.size(); i++)
	{
		if(cmd[i] == ';' && j == 0)
		{
			j++;
		}
		else if(cmd[i] == ';' && j != 0)
		{
			// remember to add tag to clientside GameItem
			gItem.emplace_back(GameItem(id, strTo<float>::value(x), strTo<float>::value(y), strTo<float>::value(z), tag));
			id = "";
			x = "";
			y = "";
			z = "";
			tag = "";
		}

		// get the map command body
		else if(j == 0)
		{
			body += cmd[i];
		}
		else if(cmd[i] == ':')
		{
			k++;
		}
		else
		{
			switch(k)
			{
				case 0: id += cmd[i]; break;
				case 1: x += cmd[i]; break;
				case 2: y += cmd[i]; break;
				case 3: z += cmd[i]; break;
				case 4: tag += cmd[i]; break;
				case 5: k = 0; break;
			}
		}
	}
	
	return body;
}

bool Room::updatePlayer(const std::string& cmd, const Connection& cnn)
{
	int n = 0;
	std::string arg, client, px, py, pz, ry;

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
				case 5: ry += cmd[i]; break;
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

	ss << arg << ":" << client << ":" << px << ":" << py << ":" << pz << ":" << ry;
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

