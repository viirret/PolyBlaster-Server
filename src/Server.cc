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
				std::stringstream received(msg->get_payload());

				std::string cmd;
				received >> cmd;
				
				std::cout << cmd << std::endl;

				if(subStr(cmd, 4) == "join")
				{
					server.send(cnn, "join", websocketpp::frame::opcode::text);

					return;
				}
				else if(subStr(cmd, 3) == "pos")
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
			catch (websocketpp::exception const &e)
			{
				std::cout << "Echo failed because: " << e.what() << std::endl;
			}
		});

		server.set_open_handler([this](Connection cnn)
		{
			connections.insert(cnn);
		});

		server.set_close_handler([this](Connection cnn)
		{
			connections.erase(cnn);
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


std::string Server::subStr(const std::string& str, int n)
{
	if(str.length() < n)
		return str;
	return str.substr(0, n);
}

bool Server::positionVector(const std::string& cmd)
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

