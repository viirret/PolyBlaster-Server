#ifndef GAMEITEM_HH
#define GAMEITEM_HH

#include <string>
#include <sstream>

struct GameItem
{
	GameItem(std::string id, float x, float y, float z, std::string tag);

	// print all fields of item
	std::string print();

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
