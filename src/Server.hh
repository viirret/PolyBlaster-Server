#ifndef SERVER_HH
#define SERVER_HH

#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"

#include <string>
#include <set>

typedef websocketpp::server<websocketpp::config::asio> Websocket;
typedef websocketpp::connection_hdl Connection;
typedef std::set<Connection, std::owner_less<Connection>> ConnectionList;
typedef Websocket::message_ptr Message;

class Server
{
	public:
		Server(int argv, char** argc);
	private:
		Websocket server;
		ConnectionList connections;

		int argv;
		char** argc;

		std::string subStr(const std::string& str, int n);
		bool positionVector(const std::string& cmd);
};

#endif
