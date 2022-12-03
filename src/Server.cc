#include "Server.hh"

#include <iostream>
#include <thread>

Server::Server(int argc, char** argv) : argc(argc), argv(argv)
{
	// handle command line arguments as custom port number
	if(argv[1])
	{
		std::stringstream ss;
		ss << argv[1];
		ss >> port;

		std::cout << "Opening server with command line argument port: " << port << std::endl;
	}
	else
	{
		std::cout << "Opening server with default port: " << port << std::endl;
	}

	try
	{
		// initialize websocket
		server.set_access_channels(websocketpp::log::alevel::all);
		server.clear_access_channels(websocketpp::log::alevel::frame_payload);
		server.init_asio();
	}
	catch(const std::exception& e)
	{
		std::cout << "Couldn't start connection because: " << e.what() << std::endl;
	}

	server.set_message_handler([this](Connection cnn, Message msg)
	{
		try
		{
			std::string cmd = msg->get_payload();

			// this is the incoming message
			//std::cout << cmd << std::endl;
			// right here more optimizations could be made if needed

			// messages that are send in a room are handled by room
			Room* room = findRoom(cnn);
			if(room)
			{
				room->handleMessage(cnn, cmd);
				
				// if player left to lobby added connection again	
				Connection* c = room->leftRoom();
				if(c)
					connections.insert(*c);

				return;
			}
			
			// get the identifier from the command
			std::string fw;
			for(std::string::size_type i = 0; i < cmd.size(); i++)
			{
				if(cmd[i] == ':') break;
				else fw += cmd[i];
			}

			switch(commands[fw])
			{
				case cmd::join:
				{
					int n = 0;
					std::string roomName, playerID, username;
					for(std::string::size_type i = 0; i < cmd.size(); i++)
					{
						if(cmd[i] == ':')
							n++;
						else
						{
							switch(n)
							{
								case 0: break;
								case 1: roomName += cmd[i]; break;
								case 2: playerID += cmd[i]; break;
								case 3: username += cmd[i]; break;
							}
						}
					}
					
					auto it = rooms.find(roomName);

					// player is already in a room
					if(it == rooms.end())
					{
						return;
					}
					
					// change the connection from lobby to Room
					removeLobbyConnection(cnn);
					it->second.addConnection(cnn, playerID, username);

					return;
				}

				case cmd::deleteRoom:
				{
					std::cout << "DELETEROOM called" << std::endl;

					int n = 0;
					std::string ID, roomID;

					for(std::string::size_type i = 0; i < cmd.size(); i++)
					{
						if(cmd[i] == ':')
							n++;
						else
						{
							switch(n)
							{
								case 0: break;
								case 1: ID += cmd[i]; break;
								case 2: roomID += cmd[i]; break;
							}
						}
					}

					// loop all existing rooms
					for(auto& r : rooms)
					{
						// find correct room
						if(r.first == roomID)
						{
							std::cout << "CORRECT ROOM FOUND" << std::endl;
							rooms.erase(r.first);
						}
					}

					return;

				}

				case cmd::list:
				{
					std::ostringstream roomData;
					roomData << "list";
 
					for(auto& room : rooms)
						roomData << ":" << room.first << ":" << room.second.getStatus().str() << ";";

					server.send(cnn, roomData.str(), websocketpp::frame::opcode::text);
					return;
				}

				case cmd::create:
				{
					int n = 0;
					std::string id, playerID, max, mode, friendlyFire, scorethreshold, warmup, itemMap, username;
					for(std::string::size_type i = 0; i < cmd.size(); i++)
					{
						if(cmd[i] == ':')
							n++;
						else
						{
							switch(n)
							{
								case 0: break;
								case 1: id += cmd[i]; break;
								case 2: playerID += cmd[i]; break;
								case 3: max += cmd[i]; break;
								case 4: mode += cmd[i]; break;
								case 5: friendlyFire += cmd[i]; break;
								case 6: scorethreshold += cmd[i]; break;
								case 7: warmup += cmd[i]; break;
								case 8: itemMap += cmd[i]; break;
								case 9: username += cmd[i]; break;
								default: std::cout << "Something went wrong!" << std::endl;
							}
						}
					}

					if(id.empty())
					{
						return;
					}

					if(rooms.find(id) != rooms.end())
					{
						server.send(cnn, "room-exists", websocketpp::frame::opcode::text);
						return;
					}

					// create room
					auto room = rooms.emplace(id, Room(server, playerID, strTo<int>::value(max), static_cast<GameMode>(strTo<int>::value(mode)),
					strTo<int>::value(friendlyFire) == 1, strTo<int>::value(scorethreshold), strTo<int>::value(warmup), strTo<int>::value(itemMap)));

					// start updating room
					room.first->second.update();

					// change connection from lobby to room
					removeLobbyConnection(cnn);
					room.first->second.addConnection(cnn, playerID, username);

					// info other clients of new room
					broadcast("newroom");

					// get information from client about newly created room
					server.send(cnn, "mapinfo:" + itemMap, websocketpp::frame::opcode::text);

					return;
				}
			}
			
			// inform client of faulty command
			server.send(cnn, "invalid:" + cmd, websocketpp::frame::opcode::text);
		}	

		catch(const std::exception& e)
		{
			std::cout << "ERROR:" << e.what() << std::endl;
		}
		catch(...)
		{
			std::cout << "ERROR" << std::endl;
		}
	});


	server.set_open_handler([this](Connection cnn)
	{
		connections.insert(cnn);
	});

	server.set_close_handler([this](Connection cnn)
	{
		Room* room = findRoom(cnn);

		// remove from Rooms or Servers connections
		room ? room->leaveRoom(cnn) : removeLobbyConnection(cnn);
	});

	server.set_reuse_addr(true);
	server.listen(port);
	server.start_accept();
	std::cout << "Server running on port: " << port << std::endl;

	std::thread updatePlayerCount([this]()
	{
		for(;;)
		{
			// connections not in a room
			int amount = connections.size();
			
			// connections from all lobbies
			for(auto& r : rooms)
			{
				amount += r.second.getConnections().size();
			}

			// amount of connections changed
			if(amount != allConnections)
			{
				std::string msg = "updateConnections:" + std::to_string(amount);
				broadcast(msg);
				allConnections = amount;
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	});

	updatePlayerCount.detach();

	server.run();
}

void Server::removeLobbyConnection(Connection cnn)
{
	connections.erase(connections.find(cnn));
}

void Server::broadcast(const std::string& msg)
{
	for(auto& c : connections)
		server.send(c, msg, websocketpp::frame::opcode::text);
}

Room* Server::findRoom(Connection& cnn)
{
	for(auto& room : rooms)
		if(room.second.connectionHere(cnn))
			return &room.second;

	return nullptr;
}

Player* Server::findPlayer(Connection& cnn)
{
	Room* room = findRoom(cnn);
	for(auto& c : room->getConnections())
		if(Util::equals(c.first, cnn))
			return &c.second;

	return nullptr;
}


