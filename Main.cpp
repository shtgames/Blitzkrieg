#include "Backend.h"
#include "Frontend.hpp"

#include <GUI/AudioSystem.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

void main()
{
	fEnd::Resources::load();

	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Blitzkrieg: The Thousand-Year Reich", sf::Style::Fullscreen);
	fEnd::setIcon(window);
	fEnd::GameInterface interface(window.getSize());

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

	bEnd::loadSavedGame("save game/The Road to War.bk");
	bEnd::ResourceDistributor::get(bEnd::Tag("BUL")).update();
	fEnd::Map::updateAllRegionColors();
	interface.updatePlayer();
	
	sf::Event event;
	while (true)
	{
		while (window.pollEvent(event))
			interface.input(event);

		window.clear(sf::Color(128, 128, 128, 255));
		
		window.draw(interface);
		window.display();
	}
}