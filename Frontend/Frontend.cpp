#include "../Frontend.hpp"

#include "../Backend/FileProcessor.h"

#include <iostream>

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

	void Resources::load()
	{
		for (const auto& it : bEnd::getDirectoryContents("resources/fonts/*.ttf"))
			fonts[it.substr(0, it.size() - 4)].loadFromFile("resources/fonts/" + it);

		for (const auto& it : bEnd::getDirectoryContents("resources/textures/*.png"))
			textures[it.substr(0, it.size() - 4)].loadFromFile("resources/textures/" + it);
	}
}