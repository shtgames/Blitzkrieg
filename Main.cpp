#include "Backend.h"
#include "Frontend.hpp"

void main()
{
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Blitzkrieg: The Thousand-Year Reich", sf::Style::Fullscreen);

	fEnd::Region::loadRegionDefinitions();
	fEnd::loadInterface(window.getSize());
	bEnd::load();

	while (true)
	{

	}
}