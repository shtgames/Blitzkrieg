#pragma once

#include <atomic>
#include <vector>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

#include "utilities.h"

namespace fEnd
{
	class Province
	{
		friend class Map;
		friend class MapLoader;
	public:
		std::string name;
		std::atomic<bool> sea = false, highlighted = false;
	private:
		void traceShape(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
			std::vector<sf::Vector2s>& borderTrianglesTarget, std::vector<std::vector<sf::Vector2s>>& contourPointsTarget);

		std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> indexRange;
		sf::FloatRect bounds;
		std::atomic<bool> visible = false;
	};
}