#ifndef PLAYER_HH
#define PLAYER_HH

#include <string>

class Player
{
	public:
		Player(std::string playerID, std::string username);
		std::string getId();
		std::string getPos();
		std::string getUsername();

		void changePos(float x, float y, float z);

		void setTeam(int team);
		int getTeam();
	private:
		float x = 0.0f, y = 0.0f, z = 0.0f;
		std::string playerID;
		std::string username;
		int team = -1;
};

#endif
