#pragma once

#include <vector>
#include<map>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include "utilities.h"

namespace fEnd
{
	class MapLoader
	{
		friend class Map;

		static void loadProvinces();
		static void createProvinceCache();
		static void loadProvinceCache();
		static void loadResources();
		static void loadProvinceNames();
		static void generateWorldGraph();

		static void connectProvinces(const unsigned short a, const unsigned short b);
		static void createStripesTexture(sf::Texture& targetTexture, const float size = 64.0f, const float stripeWidth = 0.4f);
		static void assignBorderTriangles(std::vector<sf::Vector2s>& unassignedTriangles,
			std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>>& provinceContours);
	};
}