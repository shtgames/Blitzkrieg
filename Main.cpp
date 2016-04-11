#include "Backend.h"
#include "Frontend.hpp"

#include <GUI/WindowManager.h>

#include <iostream>
#include <Windows.h>

void main()
{
	fEnd::Map::loadRegions();

	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Blitzkrieg: The Thousand-Year Reich", sf::Style::Fullscreen);
	
	fEnd::Map::loadResources();
	fEnd::Map::updateRegionVisuals(sf::Vector2s(window.getSize().x, window.getSize().y));

	gui::Window map;
	map.add(fEnd::Map());

	while (true)
	{
		sf::Event event;
		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed) std::exit(0);
			else map.input(event);

		window.clear(sf::Color(128, 128, 128, 255));
		window.draw(map);
		window.display();
	}
}