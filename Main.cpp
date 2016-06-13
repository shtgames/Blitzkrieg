#include "Backend.h"
#include "Frontend.hpp"

#include <GUI/AudioSystem.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

void main()
{
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Blitzkrieg: The Thousand-Year Reich", sf::Style::Fullscreen);
	fEnd::initializeWindow(window);

	std::atomic<bool> loading(true);
	window.setActive(false);
	std::thread loadingScreen([&window, &loading] () { fEnd::drawLoadingScreen(window, loading); });

	fEnd::Resources::load();

	fEnd::GameInterface gameInterface(window.getSize());

	bEnd::loadSavedGame("save game/War in the West.bk"); // Here for testing purposes only.
	fEnd::Map::updateAllRegionColors(); //
	gameInterface.updatePlayer(); //

	std::thread updateThread([]() { while (true) bEnd::TimeSystem::update(); });

	loading = false;
	loadingScreen.join();
	window.setActive(true);

	sf::Event event;
	while (true)
	{
		while (window.pollEvent(event)) gameInterface.input(event);

		window.clear(sf::Color(117, 121, 126, 255));
		window.draw(gameInterface);
		window.display();
	}
}