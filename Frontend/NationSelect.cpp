#include "../Frontend.hpp"

#include "NationSelect.h"

#include "../Backend.h"

#include <GUI/Button.h>

namespace fEnd
{
	NationSelectScreen::NationSelectScreen(const sf::Vector2u& resolution)
	{
		bEnd::loadSavedGame("save game/" + bEnd::getDirectoryContents("save game/*.bk").at(0));
		fEnd::Map::updateAllRegionColors();
		main.emplace("HUD", gui::Window()
				.add("Play", gui::Button(gui::Icon(Resources::texture("play_button"), true))
					.setName(gui::TextArea("Start", Resources::font("arial"), 15).setPosition(0, -5).setColor(sf::Color(200, 200, 200)))
					.setPredicates(gui::Button::PredicateArray{ []() { return bool(Map::target); } })
					.setPredicateMessage(gui::HoverMessage(gui::bind("Select a nation to continue."), Resources::font("arial"), 13)
						.setBackgroundFill(sf::Color(30, 30, 35, 240))
						.setBorderThickness(2)
						.setBorderFill(sf::Color(45, 45, 50, 245)))
					.setMessage(gui::HoverMessage(gui::bind("") + []()
						{
							if (!Map::target) return gui::bind("");
							return gui::bind("Play as ") + gui::bind(Nation::get(bEnd::Region::get(*Map::target).getController()).getName(), sf::Color(231, 194, 18)) + gui::bind(".");
						}, Resources::font("arial"), 13)
							.setBackgroundFill(sf::Color(30, 30, 35, 240))
							.setBorderThickness(2)
							.setBorderFill(sf::Color(45, 45, 50, 245)))
					.setPosition(resolution.x - Resources::texture("play_button").getSize().x, resolution.y - Resources::texture("play_button").getSize().y))
				.add("back", gui::Button(gui::Icon(Resources::texture("button_small"), true))
					.setName(gui::TextArea("Back", Resources::font("arial"), 15).setPosition(0, -5).setColor(sf::Color(200, 200, 200)))
					.setPosition(0, resolution.y - Resources::texture("button_small").getSize().y)
					.bindAction(gui::Released, []()
						{
							Map::deselect();
							fEnd::currentScreen = Menu;
						})), true)
			.emplace("Map", gui::Window().add("Map", Map()), true);
	}

	void NationSelectScreen::run(sf::RenderWindow& target, GameInterface& gameInterface)
	{
		((gui::Button&)main.at("HUD", true).at("Play"))
			.bindAction(gui::Released, [&target, &gameInterface]()
				{
					Map::deselect();
					currentScreen = Game;
					gameInterface.updatePlayer();
					std::atomic<bool> running(true);
					std::thread updateThread([&running]() { while (running) bEnd::TimeSystem::update(); });
					sf::Event event;
					while (currentScreen == Game)
					{
						while (target.pollEvent(event))
							if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::End)
							{
								Map::deselect();
								currentScreen = Menu; // temporary
							}
							else gameInterface.input(event);

							target.clear(sf::Color(117, 121, 126, 255));
							target.draw(gameInterface);
							target.display();
					}
					bEnd::TimeSystem::pause();
					running = false;
					updateThread.join();
				});

		fEnd::currentScreen = NationSelect;
		sf::Event event;
		while (currentScreen == NationSelect)
		{
			while (target.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					Map::terminate();
					std::exit(0);
				}
				if (event.type == sf::Event::MouseMoved)
				{
					fEnd::cursor.setPosition(event.mouseMove.x, event.mouseMove.y);
					main.at("Map", true).input(event);
				}
				main.input(event);
			}
			target.clear(sf::Color(117, 121, 126, 255));
			target.draw(main);
			target.draw(fEnd::cursor);
			target.display();
		}
	}
}
