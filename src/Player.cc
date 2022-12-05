#include "Player.hh"

Player::Player(std::string playerID, std::string username) : playerID(playerID), username(username)
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
void Player::setId(std::string id) { this->playerID = id; }

int Player::getTeam() { return team; }
std::string Player::getUsername() { return username; }

// kills and deaths
void Player::addKill() { kills++; }
void Player::addDeath() { deaths++; }
