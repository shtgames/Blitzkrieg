#pragma once

#include "Frontend/Map.h"
#include "Frontend/Nation.h"
#include "Frontend/GameInterface.h"
#include "Frontend/Minimap.h"
#include "Frontend/Topbar.h"
#include "Frontend/RegionPanel.h"
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
	void initializeWindow(sf::RenderWindow& window);
	void drawLoadingScreen(sf::RenderWindow& target, std::atomic<bool>& loading);
	void run();

	class Resources final
	{
	public:
		static const sf::Texture& texture(const std::string& key);
		static const bool textureExists(const std::string& key);
		static const sf::Font& font(const std::string& key);
		static void load();

	private:
		static std::unordered_map<std::string, sf::Texture> textures;
		static std::unordered_map<std::string, sf::Font> fonts;
	};
}