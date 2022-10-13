#ifndef PLAYER_HH
#define PLAYER_HH

#include <string>

class Player
{
	public:
		Player(std::string playerID);
		std::string getId();
		std::string getPos();

		void changePos(float x, float y, float z);

		void setTeam(int team);
		int getTeam();
	private:
		float x = 0.0f, y = 0.0f, z = 0.0f;
		std::string playerID;
		int team = -1;
};

#endif
