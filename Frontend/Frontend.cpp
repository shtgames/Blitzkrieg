#include "../Frontend.hpp"

#include "../Backend/FileProcessor.h"
#include "../Backend/Unit.h"

#include <GUI/AudioSystem.h>

namespace fEnd
{
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

	void drawLoadingScreen(sf::RenderWindow& target, volatile const std::atomic<bool>& loading)
	{
		sf::Texture backgroundTex, circleTex;
		backgroundTex.loadFromFile("ls/background.png");
		circleTex.loadFromFile("ls/loading.png");

		sf::View view;
		view.setCenter(1920 / 2, 1080 / 2);
		view.setSize(1920, 1080);
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

		while (loading)
		{
			target.setView(view);
			target.draw(background);
			target.setView(target.getDefaultView());
			target.draw(circle);
			target.display();
		}
	}

	void Resources::load(sf::RenderTarget& window)
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
			gui::AudioSystem::setMasterVolume(100);
		}

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
}