#pragma once

#include "Frontend/Map.h"
#include "Frontend/Nation.h"
#include "Frontend/GameInterface.h"
#include "Frontend/Minimap.h"
#include "Frontend/Topbar.h"
#include "Frontend/ProvincePanel.h"
#include "Frontend/NationSelect.h"
#include "Frontend/Console.h"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>

#include <unordered_map>

namespace fEnd
{
	enum Screen
	{
		Menu,
		NationSelect,
		Game
	};
	extern volatile Screen currentScreen;
	extern sf::Sprite cursor;
	extern Console console;
	extern const unsigned short menuFramerateCap, ingameFramerateCap;
	void initializeWindow(sf::RenderWindow& window);
	void drawLoadingScreen(sf::RenderWindow& target, std::atomic<bool>& loading);
	int run();

	class Resources final
	{
	public:
		static const bool loadTexture(const std::string& key, const std::string& path, const bool smooth = false, const bool repeated = false);
		static sf::Texture& texture(const std::string& key);
		static const bool textureExists(const std::string& key);

		static const bool loadFont(const std::string& key, const std::string& path);
		static const sf::Font& font(const std::string& key);
		static const bool fontExists(const std::string& key);

		static void load(const sf::Vector2u& resolution);

	private:
		static std::unordered_map<std::string, std::shared_ptr<sf::Texture>> textures;
		static std::unordered_map<std::string, std::shared_ptr<sf::Font>> fonts;
		static std::mutex texturesLock, fontsLock;
	};
}