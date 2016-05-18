#ifndef UTILITIES
#define UTILITIES

#include <vector>
#include <SFML/Graphics.hpp>
#include <memory>
#include <set>

namespace sf
{
	typedef Vector2<short>(Vector2s);
}

namespace utl
{
	const bool linesIntersect(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& c, const sf::Vector2f& d);
	const bool pointIsInsidePolygon(const sf::VertexArray& points, const unsigned int indexBegin, const unsigned int indexEnd, const sf::Vector2f& point);
	const bool pointIsInsideTriangle(const sf::Vector2s& A, const sf::Vector2s& B, const sf::Vector2s& C, const sf::Vector2s& point);
	std::vector<std::vector<sf::Color>> imageToPixelArray(const sf::Image&);
	const bool pointsAreOnOneLine(const sf::Vector2s&, const sf::Vector2s&, const sf::Vector2s&);
	const char angleType(const sf::Vector2s&, const sf::Vector2s&, const sf::Vector2s&);
	void cullBorderTriangles(const std::vector<std::vector<sf::Color>>&, const sf::Color&, std::vector<sf::Vector2s>&, std::vector<sf::Vector2s>&);
	void floodFillColorChange(std::vector<std::vector<sf::Color>>&, unsigned short, unsigned short, const sf::Color&, const sf::Color&);
	const sf::Vector2s findStartingPixel(const std::vector<std::vector<sf::Color>>&, const sf::Color&);
	const std::vector<std::vector<sf::Vector2s>> marchingSquares(std::vector<std::vector<sf::Color>>&, const sf::Color&);
	const std::vector<sf::Vector2s> simplifyShape(const std::vector<sf::Vector2s>&);
	void tesselateShape(const std::vector<sf::Vector2s>&, std::vector<sf::Vector2s>&);
};

#endif