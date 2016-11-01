#include "../Frontend.hpp"
#include "../Backend.h"

#include "../Backend/FileProcessor.h"
#include "../Backend/Unit.h"

#include <GUI/Button.h>
#include <GUI/AudioSystem.h>
#include <GUI/Background.h>

#include <boost/thread.hpp>

namespace fEnd
{
	constexpr auto version = "BkTYR: Version 2.3.4 (Alpha), 22 October 2016";

	volatile Screen currentScreen = Menu;
	sf::Sprite cursor;
	Console console;
	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> Resources::textures;
	std::unordered_map<std::string, std::shared_ptr<sf::Font>> Resources::fonts;
	std::mutex Resources::texturesLock, Resources::fontsLock;
	const unsigned short menuFramerateCap = 60, ingameFramerateCap = 0;

	const bool Resources::loadFont(const std::string& key, const std::string& path)
	{
		std::lock_guard<std::mutex> guard(fontsLock);
		fonts[key].reset(new sf::Font());
		if (!fonts.at(key)->loadFromFile(path))
		{
			fonts.erase(key);
			return false;
		}
		return true;
	}

	const sf::Font& Resources::font(const std::string& key)
	{
		std::lock_guard<std::mutex> guard(fontsLock);
		return *fonts.at(key);
	}

	const bool Resources::fontExists(const std::string& key)
	{
		std::lock_guard<std::mutex> guard(fontsLock);
		return fonts.count(key);
	}

	const bool Resources::loadTexture(const std::string& key, const std::string& path, const bool smooth, const bool repeated)
	{
		std::lock_guard<std::mutex> guard(texturesLock);
		textures[key].reset(new sf::Texture());
		if (!textures.at(key)->loadFromFile(path))
		{
			textures.erase(key);
			return false;
		}
		textures.at(key)->setSmooth(true);
		textures.at(key)->setRepeated(repeated);
		return true;
	}

	sf::Texture& Resources::texture(const std::string& key)
	{
		std::lock_guard<std::mutex> guard(texturesLock);
		return *textures.at(key);
	}

	const bool Resources::textureExists(const std::string& key)
	{
		std::lock_guard<std::mutex> guard(texturesLock);
		return textures.count(key);
	}

	void drawLoadingScreen(sf::RenderWindow& target, std::atomic<bool>& loading)
	{
		target.setActive(true);

		gui::Background background(Resources::texture("loading_screen_bg"));
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
		} circle(Resources::texture("loading_screen_circle"), target);

		target.setFramerateLimit(menuFramerateCap);
		
		//sf::Event event;
		while (loading)
		{
			/*while (target.pollEvent(event))
			{
				if (event.type == sf::Event::Closed) std::exit(0);
				else if (event.type == sf::Event::MouseMoved)
					cursor.setPosition(event.mouseMove.x, event.mouseMove.y);
				console.input(event);
			}*/

			target.draw(background);
			target.draw(circle);
			target.draw(console);
			//target.draw(cursor);

			target.display();
		}

		target.setFramerateLimit(ingameFramerateCap);
		target.setActive(false);
	}

	std::unique_ptr<GameInterface> gameInterface;
	std::unique_ptr<NationSelectScreen> nationSelect;
	gui::Window menu;
	std::atomic<bool> loading(true);

	void Resources::load(const sf::Vector2u& resolution)
	{
		const auto startPoint(gui::Internals::timeSinceStart());

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

		fontsLock.lock();
		for (const auto& it : bEnd::getDirectoryContents("resources/fonts/*.ttf"))
		{
			const auto key(it.substr(0, it.size() - 4));
			fonts[key].reset(new sf::Font());
			if (!fonts.at(key)->loadFromFile("resources/fonts/" + it))
				fonts.erase(key);
		}
		fontsLock.unlock();

		console.init();
		
		texturesLock.lock();
		for (const auto& it : bEnd::getDirectoryContents("resources/textures/*.png"))
		{
			const auto key(it.substr(0, it.size() - 4));
			textures[key].reset(new sf::Texture());
			if (!textures.at(key)->loadFromFile("resources/textures/" + it))
				textures.erase(key);
		}
		texturesLock.unlock();

		Nation::loadNations();
		Map::initialise();
		bEnd::Unit::load();

		console.print("Loading interface.");

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
			.add("ver", gui::TextArea(version, Resources::font("arial"), 15)
				.setOutlineThickness(2)
				.setOutlineColor(sf::Color(15, 10, 30))
				.setColor(sf::Color(160, 160, 165)));

		menu.at("ver").setPosition(resolution.x - menu.at("ver").getGlobalBounds().width - 10, resolution.y - menu.at("ver").getGlobalBounds().height - 10);
		
		loading = false;

		const unsigned short minutes(int(gui::Duration(gui::Internals::timeSinceStart() - startPoint).count()) / 60),
			seconds(int(gui::Duration(gui::Internals::timeSinceStart() - startPoint).count()) % 60);

		console.print("Done: load took " + (minutes <= 9 ? std::string("0") : std::string("")) + std::to_string(minutes) + ":" +
			(seconds <= 9 ? std::string("0") : std::string("")) + std::to_string(seconds));
	}

	void initializeWindow(sf::RenderWindow& window)
	{
		window.setMouseCursorVisible(false);

		sf::Image icon;
		icon.loadFromFile("Icon.png");
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	}

	int run()
	{
		sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Blitzkrieg: The Thousand-Year Reich", sf::Style::Fullscreen);
		fEnd::initializeWindow(window);

		{
			sf::Texture* cursorTex(new sf::Texture());
			cursorTex->loadFromFile("resources/cursor.png");
			fEnd::cursor.setTexture(*cursorTex);

			Resources::loadTexture("loading_screen_bg", "ls/background.png", true);
			Resources::loadTexture("loading_screen_circle", "ls/loading.png", true);
		}

		window.setActive(false);

		boost::thread loadingScreenThread([&]() { fEnd::drawLoadingScreen(window, loading); });

		fEnd::Resources::load(window.getSize());
		loadingScreenThread.join();

		((gui::Button&)menu.at("singleplayer")).bindAction(gui::Released, [&window]()
			{
				if (!nationSelect || !gameInterface) return;
				window.setFramerateLimit(ingameFramerateCap);
				nationSelect->run(window, *gameInterface);
				window.setFramerateLimit(menuFramerateCap);
			});

		console.allowInput(true);

		Resources::texture("main_menu_bg").setSmooth(true);
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

		return 1889;
	}
}
