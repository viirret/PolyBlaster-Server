#ifndef PLAYER_HH
#define PLAYER_HH

#include <string>

class Player
{
	public:
		Player(std::string playerID);
	private:
		void changePos(float x, float y, float z);
		float x = 0.0f, y = 0.0f, z = 0.0f;
		std::string playerID;
		float CM = 5.0f;
};

#endif
