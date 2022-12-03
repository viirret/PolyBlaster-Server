#ifndef PLAYER_HH
#define PLAYER_HH

#include <string>

class Player
{
	public:
		Player(std::string playerID, std::string username);

		void setId(std::string id);
		std::string getId();
		
		std::string getPos();
		std::string getUsername();
		int getKills();
		int getDeaths();

		void addKill();
		void addDeath();

		void changePos(float x, float y, float z);

		void setTeam(int team);
		int getTeam();
	private:
		float x = 0.0f, y = 0.0f, z = 0.0f;
		std::string playerID;
		std::string username;
		int team = -1;

		// counting for every players starts at 0 
		// because we don't store values outside Room context
		int kills = 0;
		int deaths = 0;
};

#endif
