#include "Player.hh"

Player::Player(std::string playerID) : playerID(playerID)
{
}


void Player::changePos(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

std::string Player::getPos()
{
	return std::to_string(x) + ":" + std::to_string(y) + ":" + std::to_string(z);
}

void Player::setTeam(int team) { this->team = team; }

std::string Player::getId() { return playerID; }
int Player::getTeam() { return team; }
