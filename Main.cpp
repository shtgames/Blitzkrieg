#include "Backend.h"
#include "Frontend.hpp"

#include <GUI/AudioSystem.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

void main()
{
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Blitzkrieg: The Thousand-Year Reich", sf::Style::Fullscreen);
	fEnd::initializeWindow(window);

	std::atomic<bool> loading;
	window.setActive(!(loading = true));
	std::thread loadingScreen([&window, &loading] () { fEnd::drawLoadingScreen(window, loading); });
	loadingScreen.detach();

	fEnd::Resources::load();

	fEnd::GameInterface interface(window.getSize());
	//
	bEnd::loadSavedGame("save game/The Road to War.bk");
	fEnd::Map::updateAllRegionColors();
	interface.updatePlayer();
	//
	std::thread updateThread([]()
	{
		while (true) bEnd::TimeSystem::update();
	});
	updateThread.detach();

	loading = false;
	while (!loading);
	window.setActive(true);
	window.setVerticalSyncEnabled(true);

	sf::Event event;
	while (true)
	{
		while (window.pollEvent(event)) interface.input(event);

		window.clear(sf::Color(117, 121, 126, 255));
		window.draw(interface);
		window.display();
	}
}