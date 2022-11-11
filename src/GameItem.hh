#ifndef GAMEITEM_HH
#define GAMEITEM_HH

#include <string>

struct GameItem
{
	GameItem(std::string id, float x, float y, float z, std::string tag);

	// unique id for every item
	std::string id;

	// item's position
	float x;
	float y;
	float z;

	// item's Unity tag
	std::string tag;
};

#endif
