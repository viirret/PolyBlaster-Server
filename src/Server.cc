#include "Server.hh"

#include <iostream>

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
					std::string id, playerID;
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
					}

					if(id.empty())
						return;

					if(rooms.find(id) != rooms.end())
					{
						server.send(cnn, "room-exists", websocketpp::frame::opcode::text);
						return;
					}

					auto room = rooms.emplace(id, Room(server));
					room.first->second.addConnection(cnn, playerID);
					return;
				}

				else if(cmd == "list")
				{
					std::ostringstream roomData;
					roomData << "list";

					for(auto& room : rooms)
						roomData << room.first << ":" << room.second.getStatus().str() << ";";

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

		unsigned port = 8080;
		server.listen(port);
		server.start_accept();
		std::cout << "Server running on port: " << port << std::endl;

		// if need new threads, create here

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

Room* Server::findRoom(Connection& cnn)
{
	for(auto& room : rooms)
		if(room.second.connectionHere(cnn))
			return &room.second;

	return nullptr;
}


