#include "Server.hh"

#include <iostream>
#include <thread>

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
				std::cout << cmd << std::endl;

				Room* room = findRoom(cnn);
				if(room)
					room->handleMessage(cnn, cmd);

				else if(Util::subStr(cmd, 6) == "create")
				{
					int n = 0;
					std::string id, playerID, max, mode;
					for(std::string::size_type i = 0; i < cmd.size(); i++)
					{
						if(cmd[i] == ':')
						{
							n++;
						}
						else if(n <= 0)
						{
							// "create"
						}
						else if(n <= 1)
						{
							id += cmd[i];
						}
						else if(n <= 2)
						{
							playerID += cmd[i];
						}
						else if(n <= 3)
						{
							max += cmd[i];
						}
						else if(n <= 4)
						{
							mode += cmd[i];
						}
					}

					if(id.empty())
						return;

					if(rooms.find(id) != rooms.end())
					{
						server.send(cnn, "room-exists", websocketpp::frame::opcode::text);
						return;
					}

					std::stringstream ss;
					int gameMode;
					ss << mode;
					ss >> gameMode;

					std::stringstream ss2;
					int maxint;
					ss2 << max;
					ss2 >> maxint;

					auto r = Room(server, playerID, maxint, static_cast<GameMode>(gameMode));
					auto room = rooms.emplace(id, r);
					r.update();

					removeLobbyConnection(cnn);
					room.first->second.addConnection(cnn, playerID);

					for(auto& c : connections)
					{
						server.send(c, "newroom", websocketpp::frame::opcode::text);
					}

					return;
				}

				else if(cmd == "list")
				{
					std::ostringstream roomData;
					roomData << "list";

					for(auto& room : rooms)
						roomData << ":" << room.first << ":" << room.second.getStatus().str() << ";";

					server.send(cnn, roomData.str(), websocketpp::frame::opcode::text);
					return;
				}

				else if(Util::subStr(cmd, 4) == "join")
				{
					int n = 0;
					std::string roomName, playerID;
					for(std::string::size_type i = 0; i < cmd.size(); i++)
					{
						if(cmd[i] == ':')
						{
							n++;
						}
						else if(n <= 0)
						{
							// "join"
						}
						else if(n <= 1)
						{
							roomName += cmd[i];
						}
						else if(n <= 2)
						{
							playerID += cmd[i];
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

				else
				{
					server.send(cnn, "invalid", websocketpp::frame::opcode::text);
				}


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
			removeLobbyConnection(cnn);
		});

		unsigned port = 8080;
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

					for(auto& c : connections)	
					{
						server.send(c, msg, websocketpp::frame::opcode::text);
					}
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

void Server::removeLobbyConnection(Connection cnn)
{
	if(connections.find(cnn) != connections.end())
		connections.erase(cnn);
}

Room* Server::findRoom(Connection& cnn)
{
	for(auto& room : rooms)
		if(room.second.connectionHere(cnn))
			return &room.second;

	return nullptr;
}


