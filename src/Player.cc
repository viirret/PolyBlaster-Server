#include "Player.hh"

#include <cmath>

Player::Player(std::string playerID) : playerID(playerID)
{
}

bool Player::checkPos(float nx, float ny, float nz)
{
	if(x != 0.0f && y != 0.0f && z != 0.0f)
	{
		if(x - nx < std::abs(CM) && y - ny < std::abs(CM) && z - nz < std::abs(CM))
		{
			changePos(nx, ny, nz);
			return true;
		}
		return false;
	}
	else
	{
		changePos(nx, ny, nz);
		return true;
	}
}

void Player::changePos(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}
