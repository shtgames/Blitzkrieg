#ifndef UTILITIES
#define UTILITIES

#include <vector>
#include <SFML/Graphics.hpp>

namespace utl
{
	const bool threePointsMakeALine(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&);
	const char angleType(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&);
	const std::vector<sf::Vector2f> cullBorderTriangles(const std::vector<sf::Vector2f>&, sf::VertexArray&, const sf::Image&, const sf::Color&);
	void floodFillColorChange(sf::Image&, int, int, sf::Color, sf::Color);
	const sf::Vector2f findStartingPixel(const sf::Image&, const sf::Color&);
	const std::vector<std::vector<sf::Vector2f>> marchingSquares(sf::Image&, const sf::Color&);
	const std::vector<sf::Vector2f> simplifyShape(const std::vector<sf::Vector2f>&);
	const sf::VertexArray tesselateShape(const std::vector<sf::Vector2f>&);
	const sf::VertexArray pointVectorToVertexArray(const std::vector<std::vector<sf::Vector2f>>&);
	void transferTriangles(const sf::VertexArray&, sf::VertexArray&);
	const sf::Vector2f leftmostPoint(const std::vector<sf::Vector2f>&);
	enum position { LEFT, RIGHT, COLLINEAR }; const position pointPositionRelativeToLine(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&);
	const bool existsAPointToTheLeft(const sf::Vector2f&, const sf::Vector2f&, const std::vector<sf::Vector2f>&);
	const float distanceToPointSquared(const sf::Vector2f&, const sf::Vector2f&);
	const std::vector<sf::Vector2f> convexHull(const std::vector<sf::Vector2f>&);
	const bool haveMoreThanOneCommonPoint(const std::vector<sf::Vector2f>&, const std::vector<std::vector<sf::Vector2f>>&);
	const bool isPointInsideTriangle(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&);
	template <class Element> const bool arrayContainsElement(const std::vector<Element>&, const Element&);
	const std::vector<sf::Vector2f> concaveHull(const std::vector<sf::Vector2f>& tri);
};

#endif