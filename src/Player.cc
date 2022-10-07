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
