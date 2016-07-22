#include "../Frontend.hpp"
#include "../Backend.h"

#include "../Backend/FileProcessor.h"
#include "../Backend/Unit.h"

#include <GUI/Button.h>
#include <GUI/AudioSystem.h>
#include <GUI/Background.h>

namespace fEnd
{
	constexpr auto version = "BkTYR: Version 2.3.3 (Alpha), 22 July 2016";

	volatile Screen fEnd::currentScreen = Menu;
	sf::Sprite fEnd::cursor;
	Console fEnd::console;
	std::unordered_map<std::string, sf::Texture> Resources::textures;
	std::unordered_map<std::string, sf::Font> Resources::fonts;
	const unsigned short fEnd::menuFramerateCap = 60, fEnd::ingameFramerateCap = 0;

	const sf::Font& Resources::font(const std::string& key)
	{
		return fonts.at(key);
	}

	const sf::Texture& Resources::texture(const std::string& key)
	{
		return textures.at(key);
	}

	const bool Resources::textureExists(const std::string& key)
	{
		return textures.count(key);
	}

	void drawLoadingScreen(sf::RenderWindow& target, std::atomic<bool>& loading)
	{
		sf::Texture backgroundTex, circleTex;
		backgroundTex.loadFromFile("ls/background.png");
		circleTex.loadFromFile("ls/loading.png");
		circleTex.setSmooth(true);
		
		gui::Background background(backgroundTex);
		class RotateAnimation final : public gui::Animation
		{
			mutable sf::Sprite spr;
			sf::RenderWindow& window;
		public:
			RotateAnimation(const sf::Texture& texture, sf::RenderWindow& win)
				: window(win)
			{
				spr.setTexture(texture);
				spr.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
				spr.setPosition(window.getSize().x - texture.getSize().x / 2 - 25, window.getSize().y - texture.getSize().y / 2 - 25);
			}
			void draw(sf::RenderTarget& target, sf::RenderStates states) const override
			{
				Animation::draw(target, states);
				target.draw(spr);
			}
			void step()const override
			{
				spr.rotate(7);
			}
		} circle(circleTex, target);

		target.setFramerateLimit(menuFramerateCap);

		sf::Event event;
		while (loading)
		{
			while (target.pollEvent(event))
			{
				if (event.type == sf::Event::Closed) std::exit(0);
				else if (event.type == sf::Event::MouseMoved)
					cursor.setPosition(event.mouseMove.x, event.mouseMove.y);
				console.input(event);
			}

			target.draw(background);
			target.draw(circle);
			target.draw(console);
			target.draw(cursor);

			target.display();
		}

		target.setFramerateLimit(ingameFramerateCap);
	}

	std::unique_ptr<GameInterface> gameInterface;
	std::unique_ptr<NationSelectScreen> nationSelect;
	gui::Window menu;
	std::atomic<bool> loading(true);

	void Resources::load(const sf::Vector2u& resolution)
	{
		{
			unsigned char index(0);
			for (const auto& it : bEnd::getDirectoryContents("music/*.ogg"))
			{
				gui::AudioSystem::loadMusicFile(index, "music/" + it);
				index++;
			}
			gui::AudioSystem::playRandomSong();
			gui::AudioSystem::setMusicVolume(100);
		}

		{
			unsigned char index(0);
			for (const auto& it : bEnd::getDirectoryContents("sounds/*.wav"))
			{
				gui::AudioSystem::loadSoundFile(index, "sounds/" + it);
				index++;
			}
			gui::AudioSystem::setSoundVolume(100);
		}

		gui::AudioSystem::setMasterVolume(100);

		for (const auto& it : bEnd::getDirectoryContents("resources/fonts/*.ttf"))
			fonts[it.substr(0, it.size() - 4)].loadFromFile("resources/fonts/" + it);

		console.init();

		for (const auto& it : bEnd::getDirectoryContents("resources/textures/*.png"))
			textures[it.substr(0, it.size() - 4)].loadFromFile("resources/textures/" + it);

		Nation::loadNations();
		Map::initialise();
		bEnd::Unit::load();

		console.print("Loading Interface...");
		gameInterface.reset(new GameInterface(resolution));
		nationSelect.reset(new NationSelectScreen(resolution));

		menu.add("singleplayer", gui::Button(gui::Icon(Resources::texture("button_wide"), true))
			.setPosition((resolution.x - Resources::texture("button_wide").getSize().x) / 2,
				(resolution.y - Resources::texture("button_wide").getSize().y) / 2)
			.setName(gui::TextArea("Single Player", Resources::font("arial"), 15).setPosition(0, -3).setColor(sf::Color(200, 200, 200))))
			.add("exit", gui::Button(gui::Icon(Resources::texture("button_wide"), true))
				.setPosition((resolution.x - Resources::texture("button_wide").getSize().x) / 2, (resolution.y + Resources::texture("button_wide").getSize().y) / 2)
				.setName(gui::TextArea("Exit", Resources::font("arial"), 14).setPosition(0, -3).setColor(sf::Color(200, 200, 200)))
				.bindAction(gui::Released, []() { Map::terminate(); std::exit(0); }))
			.add("ver", gui::TextArea(version, Resources::font("arial"), 15).setStyle(sf::Text::Bold).setColor(sf::Color(200, 200, 200)));

		menu.at("ver").setPosition(resolution.x - menu.at("ver").getGlobalBounds().width - 10, resolution.y - menu.at("ver").getGlobalBounds().height - 10);

		loading = false;
	}

	void initializeWindow(sf::RenderWindow& window)
	{
		window.setMouseCursorVisible(false);

		sf::Image icon;
		icon.loadFromFile("Icon.png");
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	}

	void run()
	{
		sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Blitzkrieg: The Thousand-Year Reich", sf::Style::Fullscreen);
		fEnd::initializeWindow(window);
		
		std::thread loadingThread([&window]() { fEnd::Resources::load(window.getSize()); });

		{
			sf::Texture* cursorTex(new sf::Texture());
			cursorTex->loadFromFile("resources/cursor.png");
			fEnd::cursor.setTexture(*cursorTex);
		}

		fEnd::drawLoadingScreen(window, loading);				

		((gui::Button&)menu.at("singleplayer")).bindAction(gui::Released, [&window]()
			{
				if (!nationSelect || !gameInterface) return;
				window.setFramerateLimit(ingameFramerateCap);
				nationSelect->run(window, *gameInterface);
				window.setFramerateLimit(menuFramerateCap);
			});

		console.allowInput(true);

		gui::Background menuBackground(Resources::texture("main_menu_bg"));
		window.setFramerateLimit(menuFramerateCap);
		sf::Event event;
		while (true)
		{
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					Map::terminate();
					std::exit(0);
				}
				if (event.type == sf::Event::MouseMoved) fEnd::cursor.setPosition(event.mouseMove.x, event.mouseMove.y);
				console.input(event);
				menu.input(event);
			}

			window.draw(menuBackground);
			window.draw(menu);
			window.draw(console);
			window.draw(fEnd::cursor);
			window.display();
		}
	}
}