#include "Backend.h"
#include "Frontend.hpp"

#include <GUI/WindowManager.h>
#include <GUI/AudioSystem.h>
#include <GUI/FPSMeter.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

void main()
{
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Blitzkrieg: The Thousand-Year Reich", sf::Style::Fullscreen);

	{
		unsigned char index(0);
		for (auto& it : bEnd::getDirectoryContents("music/*.ogg"))
		{
			gui::AudioSystem::loadMusicFile(index, "music/" + it);
			index++;
		}
		gui::AudioSystem::playRandomSong();
		gui::AudioSystem::setMusicVolume(100);
		gui::AudioSystem::setMasterVolume(100);
	}
	
	{
		sf::Image icon;
		icon.loadFromFile("Icon.png");
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	}

	fEnd::Map::loadRegions();
	fEnd::Map::loadResources();
	fEnd::Map::updateRegionVisuals(sf::Vector2s(window.getSize().x, window.getSize().y));
	fEnd::Nation::loadNations();

	bEnd::loadSavedGame("save game/The Road to War.bk");
	fEnd::Map::updateAllRegionColors();

	fEnd::Map::launchRegionUpdateThread();

	gui::Window map;
	map.add(fEnd::Map());

	sf::Font font;
	font.loadFromFile("resources/arial.ttf");
	
	gui::FPSMeter fpsMeter(font, 25);
	fpsMeter.setColor(sf::Color(0, 255, 0))
		.setPosition(window.getSize().x - 100, 100);

	while (true)
	{
		sf::Event event;
		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed) std::exit(0);
			else map.input(event);

		window.clear(sf::Color(128, 128, 128, 255));
		
		window.draw(map);
		window.draw(fpsMeter);
		window.display();
	}
}