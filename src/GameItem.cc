#include "GameItem.hh"

GameItem::GameItem(std::string id, float x, float y, float z, std::string tag)
	: id(id), x(x), y(y), z(z), tag(tag)
{
}

std::string GameItem::print()
{
	std::stringstream ss;
	ss << id << ":" << x << ":" << y << ":" << z << ":" << tag;
	return ss.str();
}
