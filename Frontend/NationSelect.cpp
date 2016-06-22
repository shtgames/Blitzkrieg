#include "../Frontend.hpp"

#include "NationSelect.h"

#include "../Backend.h"
#include "Date.h"

#include <GUI/Button.h>

namespace fEnd
{
	class SelectedScenario final : public gui::Icon
	{
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			m_scenario.setPosition(getPosition().x + (getGlobalBounds().width - m_scenario.getGlobalBounds().width) / 2, getPosition().y + 15);

			Icon::draw(target, states);
			target.draw(m_scenario, states);
			target.draw(m_prompt, states);
		}

		mutable gui::TextPane m_scenario;
		sf::Text m_prompt;

	public:
		SelectedScenario(const SelectedScenario& copy) = default;
		SelectedScenario(SelectedScenario&& temp) = default;
		SelectedScenario(const sf::Vector2u& resolution)
			: Icon(Resources::texture("selected_scenario_bg"), true),
			m_scenario(gui::bind("") + []()
				{
					return gui::bind(bEnd::currentlyLoadedFile(), sf::Color(231, 194, 18)) + gui::bind(", " + Date::toString(bEnd::TimeSystem::getCurrentDate()), sf::Color(210, 210, 210));
				}, Resources::font("arial"), 15),
			m_prompt("Click on the map to select a nation.", Resources::font("arial"), 13)
		{
			m_prompt.setColor(sf::Color(180, 180, 180));

			setPosition((resolution.x - getGlobalBounds().width) / 2, 0);
		}
		
		std::unique_ptr<gui::Interactive> copy()const override
		{
			return std::unique_ptr<gui::Interactive>(new auto(*this));
		}
		std::unique_ptr<gui::Interactive> move() override
		{
			return std::unique_ptr<gui::Interactive>(new auto(std::move(*this)));
		}

		SelectedScenario& setPosition(const float x, const float y) override
		{
			Icon::setPosition(x, y);
			m_scenario.setPosition(x + (getGlobalBounds().width - m_scenario.getGlobalBounds().width) / 2, y + 15);
			m_prompt.setPosition(x + (getGlobalBounds().width - m_prompt.getGlobalBounds().width) / 2, y + 42);
			return *this;
		}
	};
	class SelectedNation final : public gui::Icon
	{
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!Map::target) return;
			m_flag.setTexture(Nation::get(bEnd::Province::get(*Map::target).getController()).getFlag());
			m_name.setPosition(getPosition().x + (getGlobalBounds().width - m_name.getGlobalBounds().width) / 2, 14);

			Icon::draw(target, states);
			target.draw(m_name, states);
			target.draw(m_flag, states);
			target.draw(m_flagShadow, states);
			target.draw(m_government, states);
			target.draw(m_headOfState, states);
		}

		mutable sf::Sprite m_flag;
		sf::Sprite m_flagShadow;
		mutable gui::TextArea m_name;
		gui::TextPane m_government, m_headOfState;
	public:
		SelectedNation(const SelectedNation& copy) = default;
		SelectedNation(SelectedNation&& temp) = default;
		SelectedNation(const sf::Vector2u& resolution)
			: Icon(Resources::texture("selected_bg"), true),
			m_flagShadow(Resources::texture("topbarflag_shadow")),
			m_name("", Resources::font("arial"), 13),
			m_government(gui::bind("Government: N/A\nAt Peace"), Resources::font("arial"), 10),
			m_headOfState(gui::bind("Head of State", sf::Color(231, 194, 18)) + gui::bind(":\nNone"), Resources::font("arial"), 11)
		{
			m_name.setUpdateFunction([]()
				{
					if (!Map::target) return gui::bind("");
					return gui::bind(Nation::get(bEnd::Province::get(*Map::target).getController()).getName(), sf::Color(220, 220, 220));
				});
			m_flagShadow.setScale(0.71, 0.71);
			m_flag.setScale(0.71, 0.71);
			m_flag.setRotation(90);
			m_flagShadow.setRotation(90);
			setPosition(resolution.x - getGlobalBounds().width, 0);
		}

		std::unique_ptr<gui::Interactive> copy()const override
		{
			return std::unique_ptr<gui::Interactive>(new auto(*this));
		}
		std::unique_ptr<gui::Interactive> move() override
		{
			return std::unique_ptr<gui::Interactive>(new auto(std::move(*this)));
		}

		SelectedNation& setPosition(const float x, const float y)
		{
			Icon::setPosition(x, y);
			m_name.setPosition(x + (getGlobalBounds().width - m_name.getGlobalBounds().width) / 2, 14);
			m_flagShadow.setPosition(x + m_flagShadow.getGlobalBounds().width + 26, y + 41);
			m_flag.setPosition(m_flagShadow.getPosition());
			m_government.setPosition(x + 88, y + 42);
			m_headOfState.setPosition(x + 89, y + 125);
			return *this;
		}

		const bool input(const sf::Event& event)
		{
			if (!Map::target) return false;
			return Icon::input(event);
		}
	};

	NationSelectScreen::NationSelectScreen(const sf::Vector2u& resolution)
	{
		console.print("Loading History...");
		bEnd::loadSavedGame(bEnd::getDirectoryContents("save game/*.bk").at(0));
		fEnd::Map::updateAllProvinceColors();
		console.print("Done.");

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
							return gui::bind("Play as ") + gui::bind(Nation::get(bEnd::Province::get(*Map::target).getController()).getName(), sf::Color(231, 194, 18)) + gui::bind(".");
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
							Map::waitForColorUpdate();
							fEnd::currentScreen = Menu;
						}))
				.add("current scenario", SelectedScenario(resolution))
				.add("selected nation", SelectedNation(resolution)), true)
			.emplace("Map", gui::Window().add("Map", Map()), true);
	}

	void NationSelectScreen::run(sf::RenderWindow& target, GameInterface& gameInterface)
	{
		((gui::Button&)main.at("HUD", true).at("Play"))
			.bindAction(gui::Released, [&target, &gameInterface]()
				{
					Map::deselect();
					Map::waitForColorUpdate();
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
							else
							{
								console.input(event);
								gameInterface.input(event);
							}

							target.clear(sf::Color(117, 121, 126, 255));
							target.draw(gameInterface);
							target.draw(console);
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
				console.input(event);
				main.input(event);
			}
			target.clear(sf::Color(117, 121, 126, 255));
			target.draw(main);
			target.draw(console);
			target.draw(fEnd::cursor);
			target.display();
		}
	}
}
