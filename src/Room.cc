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

void Room::addConnection(Connection& cnn, const std::string& playerID, const std::string& username)
{
	std::stringstream ss;
	std::string data;
	
	ss << "joined:" << (int)mode << ":";
	for(auto& c : connections)
	{
		ss << c.second.getUsername() << ":" << c.second.getTeam() << ";";
	}

	ss >> data;

	// inform the client of joining the game
	server.send(cnn, data, websocketpp::frame::opcode::text);

	connections.emplace(cnn, Player(playerID, username));

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
	ss << connections.size() << ":" << max << ":" << (int)mode;
	return ss;
}

void Room::handleMessage(Connection& cnn, const std::string& msg)
{
	// identifier for command, works for all commands
	std::string fw, sender;

	int parseIndex = 0;
	for(std::string::size_type i = 0; i < msg.size(); i++)
	{
		if(msg[i] == ':') 
			parseIndex++;
		else if(parseIndex < 1)
			fw += msg[i];
		else if(parseIndex < 2)
			sender += msg[i];
		else
			break;
	}

	// when executing protected commands, check that we're using our Unity id
	for(auto& p : protectedCommands)
	{
		if(fw == p)
		{
			for(auto& c : connections)
			{
				// if player tries to execute command as other player
				if(Util::equals(cnn, c.first) && sender != c.second.getId())
				{
					std::cout << "CANCELLED SPOOFED MESSAGE!" << std::endl;
					return;
				}
			}
		}
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
			broadcast(msg);
			break;
		}

		// the zones are created in Unity so this is a little trick to handle that	
		case cmd::zone:
		{
			if(zones.empty())
			{
				server.send(cnn, "getzone", websocketpp::frame::opcode::text);
			}
			return;
		}

		case cmd::getscore:
		{
			server.send(cnn, "getscore:" + std::to_string(scoreRed) + ":" + std::to_string(scoreBlue), 
				websocketpp::frame::opcode::text);
			return;
		}

		case cmd::getzone:
		{
			std::cout << "ZONES\n" << msg << std::endl;

			// set zones as serverside items
			gameItemCommand(msg, zones);

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

			// delete item from server
			for(int i = 0; i < (int)items.size(); i++)
			{
				if(items[i].id == id)
				{
					deletedItems.push_back(std::make_pair(items[i], 0));
					items.erase(items.begin() + i);
				}
			}

			// destroy item in clients
			broadcast("capture:" + id);
			
			return;
		}

		case cmd::dead:
		{
			// normal death
			int n = 0;
			std::string player, team, killer;
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
						case 3: killer += msg[i]; break;
					}
				}
			}

			// manual spoofchecking
			for(auto& c : connections)
			{
				if(!Util::equals(cnn, c.first) && player == c.second.getId())
				{
					std::cout << "SPOOFING DETECTED" << std::endl;
					return;
				}
			}

			// update server data
			for(auto& c : connections)
			{
				// the killed received death
				if(c.second.getId() == player)
				{
					c.second.addDeath();
				}

				// killer gets kill
				else if(c.second.getId() == killer)
				{
					c.second.addKill();
				}
			}

			std::stringstream ss;
			std::string deadCommand;
			ss << "dead:" << player << ":" << team << ":" << killer;
			ss >> deadCommand;

			// send message back to client confirming its death
			broadcast(deadCommand);
			
			// update scoreboard if correct teammode and it isn't suicide
			if(!(mode == GameMode::collect_items) && player != killer)
			{
				int deathTeam = strTo<int>::value(team);

				// update scores
				if(deathTeam == 0)
					scoreBlue++;
				else if(deathTeam == 1)
					scoreRed++;
			}

			// send message back to server about respawning
			server.send(cnn, "respawn", websocketpp::frame::opcode::text);

			return;
		}

		case cmd::myteam:
		{
			std::cout << "MYTEAM CALLED" << std::endl;

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
					std::cout << "PLAYER " << c.second.getId() << " KILLS " << c.second.kills << " DEATHS " << c.second.deaths << std::endl;

					info += c.second.getId();
					info += ":";
					info += c.second.getPos();
					info += ":";
					info += std::to_string(c.second.getTeam());
					info += ":";
					info += c.second.getUsername();
					info += ":";
					info += std::to_string(c.second.kills);
					info += ":";
					info += std::to_string(c.second.deaths);
					info += ";";
				}
			}
			
			server.send(cnn, info, websocketpp::frame::opcode::text);
			return;
		}

		case cmd::hardpoint:
		{
			std::cout << "HARDPOINT MESSAGE RECEIVED" << std::endl;

			int n = 0;
			std::string id, team;

			for(std::string::size_type i = 0; i < msg.size(); i++)
			{
				if(msg[i] == ':')
					n++;
				else
				{
					switch(n)
					{
						case 0: break;
						case 1: id += msg[i]; break;
						case 2: team += msg[i]; break;
					}
				}
			}

			int iteam = strTo<int>::value(team);

			if(iteam == 0)
			{
				scoreRed++;	
				hardpointScore--;
			}
			else if(iteam == 1)
			{
				scoreBlue++;
				hardpointScore--;
			}

			broadcast("hardpointScore:" + std::to_string(hardpointScore));
			return;

			// TODO add score to player that got the point
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

		case cmd::newplayer:
		{
			std::string command, id, team;
			int n = 0;

			for(std::string::size_type i = 0; i < msg.size(); i++)
			{
				if(msg[i] == ':')
					n++;
				else
				{
					switch(n)
					{
						case 0: command += msg[i]; break;
						case 1: id += msg[i]; break;
						case 2: team += msg[i]; break;
					}
				}
			}

			int kills;
			int deaths;
			std::string username;

			for(auto& c : connections)
			{
				if(c.second.getId() == id)
				{
					kills = c.second.kills;
					deaths = c.second.deaths;
					username = c.second.getUsername();
				}
			}

			std::stringstream ss;
			std::string newPlayerMessage;
			ss << command << ":" << id << ":" << team << ":" << username << ":" << kills << ":" << deaths;
			ss >> newPlayerMessage;

			broadcast(newPlayerMessage, cnn);
			
			return;
		}

		// these messages are not to be sent to itself
		case cmd::snd:
		{
			broadcast(msg, cnn);
			return;
		}

		// these messages are to be sent to every client
		case cmd::util: case cmd::chat: case cmd::asnd:
		{
			broadcast(msg);
			return;
		}

		// inform client of faulty command
		server.send(cnn, "invalid:" + msg, websocketpp::frame::opcode::text);
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
			std::this_thread::sleep_for(std::chrono::seconds(1));

			// update scores	
			if(scoreRed >= scorethreshold)
			{
				broadcast("victory:0");

				scoreRed = 0;
				scoreBlue = 0;
				warmupTime = 0;
				isWarmup = true;
			}

			if(scoreBlue >= scorethreshold)
			{
				broadcast("victory:1");

				scoreRed = 0;
				scoreBlue = 0;
				warmupTime = 0;
				isWarmup = true;
			}

			if(oldScoreRed != scoreRed || oldScoreBlue != scoreBlue)
			{
				std::string updateScore = "updatescores:" + std::to_string(scoreRed) + ":" + std::to_string(scoreBlue);

				broadcast(updateScore);

				oldScoreRed = scoreRed;
				oldScoreBlue = scoreBlue;
			}

			// warmup is going on
			if(warmupTime < (size_t)warmup)
			{
				// send message about warmup time
				warmupTime++;
				broadcast("warmup:" + std::to_string(warmupTime) + ":" + std::to_string(warmup));
			}
			// warmup has ended
			else if(warmupTime > (size_t)warmup)
			{
				isWarmup = false;

				// reset players stats
				for(auto& c : connections)
				{
					c.second.deaths = 0;
					c.second.kills = 0;
				}

				// reset scores
				scoreBlue = 0;
				scoreRed = 0;
			}
			
			// loop deleted items
			for(int i = 0; i < (int)deletedItems.size(); i++)
			{
				// check if item time exceeds 	
				if(deletedItems[i].second > recoverTime)	
				{
					broadcast("newitem:" + deletedItems[i].first.print());
					items.push_back(deletedItems[i].first);
					deletedItems.erase(deletedItems.begin() + i);
				}
				deletedItems[i].second++;
			}


			// updatating related to hardpoint
			if(mode == GameMode::hardpoint)
			{
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
					hardpointScore = originalHardPointScore;
					currentHazardIndex = Util::randomValue(0, zones.size() - 1);
				}

				std::string str = "hazard:";
				str += hazardZoneOn ? "1:" : "0:";
				str += zones[currentHazardIndex].print();

				broadcast(str);
			}

		}

	});

	updateRoom.detach();
}

bool Room::createMap(const std::string& cmd, const Connection& cnn)
{
	std::string map = gameItemCommand(cmd, items);

	// get map id from cmd
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
			k = 0;
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
			}
		}
	}
	
	return body;
}

bool Room::updatePlayer(const std::string& cmd, const Connection& cnn)
{
	int n = 0;
	std::string arg, client, px, py, pz, ry, anim;

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
				case 6: anim += cmd[i]; break;
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

	ss << arg << ":" << client << ":" << px << ":" << py << ":" << pz << ":" << ry << ":" << anim;
	ss >> command;

	float x = strTo<float>::value(px);
	float y = strTo<float>::value(py);
	float z = strTo<float>::value(pz);

	// update position of the player
	for(auto& c : connections)
		if(c.second.getId() == client)
			c.second.changePos(x, y, z);

	// here we can do some checks for the command, if needed in the future

	// send position  each client except yourself
	broadcast(command, cnn);

	return true;
}

