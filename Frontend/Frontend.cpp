#include "../Frontend.hpp"
#include "../Backend.h"

#include "../Backend/FileProcessor.h"
#include "../Backend/Unit.h"

#include <GUI/Button.h>
#include <GUI/AudioSystem.h>

namespace fEnd
{
	constexpr auto version = "BkTYR: Version 2.2.1 (Alpha)";

	volatile Screen fEnd::currentScreen = Menu;
	sf::Sprite fEnd::cursor;
	std::unordered_map<std::string, sf::Texture> Resources::textures;
	std::unordered_map<std::string, sf::Font> Resources::fonts;

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
		target.setActive(true);

		sf::Texture backgroundTex, circleTex;
		backgroundTex.loadFromFile("ls/background.png");
		circleTex.loadFromFile("ls/loading.png");
		circleTex.setSmooth(true);

		sf::View view;
		view.setCenter(backgroundTex.getSize().x / 2, backgroundTex.getSize().y / 2);
		view.setSize(sf::Vector2f(backgroundTex.getSize()));
		view.setViewport(sf::FloatRect(0, 0, 1, 1));

		sf::Sprite background(backgroundTex);
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

		target.setFramerateLimit(30);

		while (loading)
		{
			target.setView(view);
			target.draw(background);
			target.setView(target.getDefaultView());
			target.draw(circle);
			target.display();
		}

		target.setFramerateLimit(0);
		target.setActive(false);
	}

	void Resources::load()
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

		Nation::loadNations();

		for (const auto& it : bEnd::getDirectoryContents("resources/fonts/*.ttf"))
			fonts[it.substr(0, it.size() - 4)].loadFromFile("resources/fonts/" + it);

		for (const auto& it : bEnd::getDirectoryContents("resources/textures/*.png"))
			textures[it.substr(0, it.size() - 4)].loadFromFile("resources/textures/" + it);

		fEnd::Map::initialize();
		bEnd::Unit::load();
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
		
		std::atomic<bool> loading(true);
		window.setActive(false);
		std::thread loadingScreen([&window, &loading]() { fEnd::drawLoadingScreen(window, loading); });

		fEnd::Resources::load();

		fEnd::cursor.setTexture(Resources::texture("cursor"));
		fEnd::GameInterface gameInterface(window.getSize());
		fEnd::NationSelectScreen nationSelect(window.getSize());
		
		sf::Sprite bg(Resources::texture("main_menu_bg"));
		sf::View backgroundView;
		backgroundView.setCenter(Resources::texture("main_menu_bg").getSize().x / 2, Resources::texture("main_menu_bg").getSize().y / 2);
		backgroundView.setSize(sf::Vector2f(Resources::texture("main_menu_bg").getSize()));
		backgroundView.setViewport(sf::FloatRect(0, 0, 1, 1));

		gui::Window menu;
		menu.add("singleplayer", gui::Button(gui::Icon(Resources::texture("button_wide"), true))
				.setPosition((window.getSize().x - Resources::texture("button_wide").getSize().x) / 2,
					(window.getSize().y - Resources::texture("button_wide").getSize().y) / 2)
				.setName(gui::TextArea("Single Player", Resources::font("arial"), 15).setPosition(0, -3).setColor(sf::Color(200, 200, 200)))
				.bindAction(gui::Released, [&window, &nationSelect, &gameInterface]() 
					{
						nationSelect.run(window, gameInterface);
					}))
			.add("exit", gui::Button(gui::Icon(Resources::texture("button_wide"), true))
				.setPosition((window.getSize().x - Resources::texture("button_wide").getSize().x) / 2, (window.getSize().y + Resources::texture("button_wide").getSize().y) / 2)
				.setName(gui::TextArea("Exit", Resources::font("arial"), 14).setPosition(0, -3).setColor(sf::Color(200, 200, 200)))
				.bindAction(gui::Released, []() { Map::terminate(); std::exit(0); }))
			.add("ver", gui::TextArea(version, Resources::font("arial"), 15).setStyle(sf::Text::Bold).setColor(sf::Color(200, 200, 200)));
		
		menu.at("ver").setPosition(window.getSize().x - menu.at("ver").getGlobalBounds().width - 10, window.getSize().y - menu.at("ver").getGlobalBounds().height - 10);

		loading = false;
		loadingScreen.join();
		window.setActive(true);

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
				menu.input(event);
			}
			window.setView(backgroundView);
			window.draw(bg);
			window.setView(window.getDefaultView());
			window.draw(menu);
			window.draw(fEnd::cursor);
			window.display();
		}
	}
}