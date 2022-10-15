#include "Server.hh"

#include <iostream>
#include <thread>
#include <chrono>

Server::Server(int argv, char** argc) : argv(argv), argc(argc)
{
	try 
	{
		server.set_access_channels(websocketpp::log::alevel::all);
		server.clear_access_channels(websocketpp::log::alevel::frame_payload);
		server.init_asio();

		server.set_message_handler([this](Connection cnn, Message msg)
		{
			try 
			{
				std::string cmd = msg->get_payload();

				// this is the incoming message that gets resent
				// optimize sending this back
				std::cout << cmd << std::endl;

				Room* room = findRoom(cnn);
				if(room)
				{
					room->handleMessage(cnn, cmd);
					return;
				}
				
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
						std::string roomName, playerID;
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
								}
							}
						}
						
						auto it = rooms.find(roomName);

						if(it == rooms.end())
						{
							server.send(cnn, "invalid-room", websocketpp::frame::opcode::text);
							return;
						}
						
						removeLobbyConnection(cnn);
						it->second.addConnection(cnn, playerID);

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
						std::string id, playerID, max, mode, arg1, friendlyFire;
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
									case 6: arg1 += cmd[i]; break;
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

						// convert string arguments to int
						int gameMode = strTo<int>::value(mode);
						int maxint = strTo<int>::value(max);
						int intFriendlyFire = strTo<int>::value(friendlyFire);
						int arg1int = strTo<int>::value(arg1);

						bool boolFriendlyFire = intFriendlyFire == 1;

						auto room = rooms.emplace(id, Room(server, playerID, maxint, static_cast<GameMode>(gameMode), boolFriendlyFire, arg1int));
						room.first->second.update();

						removeLobbyConnection(cnn);
						room.first->second.addConnection(cnn, playerID);

						// info other clients of new room
						broadcast("newroom");

						return;
					}
				}
				
				// inform client of faulty command
				server.send(cnn, "invalid", websocketpp::frame::opcode::text);

			}
			catch (websocketpp::exception const &e)
			{
				std::cout << "Failed because: " << e.what() << std::endl;
			}
		});

		server.set_open_handler([this](Connection cnn)
		{
			connections.insert(cnn);
		});

		server.set_close_handler([this](Connection cnn)
		{
			/*
			Player* player = findPlayer(cnn);
			Room* room = findRoom(cnn);

			// TODO find out why this does not work, it should, right?
			for(auto& c : room->getConnections())
				if(!Util::equals(c.first, cnn))
					server.send(c.first, "quit:" + player->getId(), websocketpp::frame::opcode::text);
			*/

			removeLobbyConnection(cnn);
		});

		unsigned port = 8080;
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
			}
		});

		updatePlayerCount.detach();

		server.run();

	}
	
	catch (websocketpp::exception const &e)
	{
		std::cout << e.what() << std::endl;
	}

	catch (...)
	{
		std::cout << "Other exception" << std::endl;
	}
}

void Server::removeLobbyConnection(Connection& cnn)
{
	if(connections.find(cnn) != connections.end())
		connections.erase(cnn);
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


