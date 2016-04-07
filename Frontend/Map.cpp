#include "Map.h"

#include "../Backend/FileProcessor.h"
#include "../Backend/Region.h"
#include "utilities.h"

#include <fstream>
#include <sstream>
#include <set>

#include <iostream>

namespace fEnd
{
	const std::string borderShaderCode = 
		"uniform vec2 step;\
		uniform sampler2D texture;\
		uniform vec4 color;\
		\
		vec4 borderEffect()\
		{\
			float outlineIntensity = 8.0f * texture2D(texture, texturePos).a;\
			outlineIntensity -= texture2D(texture, gl_TexCoord[0].xy + vec2(step.x, 0.0f)).a;\
			outlineIntensity -= texture2D(texture, gl_TexCoord[0].xy + vec2(-step.x, 0.0f)).a;\
			outlineIntensity -= texture2D(texture, gl_TexCoord[0].xy + vec2(0.0f, step.y)).a;\
			outlineIntensity -= texture2D(texture, gl_TexCoord[0].xy + vec2(0.0f, -step.y)).a;\
			outlineIntensity -= texture2D(texture, gl_TexCoord[0].xy + vec2(step.x, step.y)).a;\
			outlineIntensity -= texture2D(texture, gl_TexCoord[0].xy + vec2(-step.x, step.y)).a;\
			outlineIntensity -= texture2D(texture, gl_TexCoord[0].xy + vec2(step.x, -step.y)).a;\
			outlineIntensity -= texture2D(texture, gl_TexCoord[0].xy + vec2(-step.x, -step.y)).a;\
		\
			return mix(texture2D(texture, gl_TexCoord[0].xy), color, outlineIntensity);\
		}\
		\
		void main()\
		{\
			vec4 borderColor = borderEffect();\
			if (borderColor == color) gl_FragColor = borderColor;\
			else gl_FragColor = gl_Color * borderColor;\
		}";

	std::unordered_map<unsigned short, Map::Region> Map::regions;

	sf::Vector2s Map::resolution;
	sf::FloatRect Map::viewBounds;
	sf::Vector2s Map::mapSize;
	sf::View Map::mapView;
	sf::RenderTexture Map::land, Map::sea;

	sf::VertexArray Map::oceanGradient, Map::provinceStripes, Map::landProvinces, Map::seaProvinces;
	sf::VertexArray Map::stripesBuffer[2], Map::landBuffer[2], Map::seaBuffer[2];
	std::atomic<bool> Map::drawableBufferSet;

	sf::Texture Map::mapTile, Map::stripes;
	std::pair<sf::Shader, sf::Vector2f> Map::border;

	void Map::loadRegions()
	{
		if (!std::ifstream("map/cache/provinces.bin").good())
		{
			unsigned short maxProvinces(0);

			{
				bEnd::FileProcessor basicRegionDefinitions;

				if (!basicRegionDefinitions.open("map/default.map")) return;

				for (auto it = basicRegionDefinitions.getStatements().begin(), end = basicRegionDefinitions.getStatements().end(); it != end; ++it)
					if (it->lValue == "map_size")
					{
						mapSize.x = std::stoi(it->rStrings.front());
						mapSize.y = std::stoi(it->rStrings.back());
					}
					else if (it->lValue == "max_provinces")
						maxProvinces = std::stoi(it->rStrings.front());
					else if (it->lValue == "sea_starts")
						for (auto it1 = it->rStrings.begin(), end1 = it->rStrings.end(); it1 != end1; ++it1)
							bEnd::Region::regions[std::stoi(*it1)].sea = true;
			}

			std::ifstream definitions("map/definition.csv");
			sf::Image provinces;

			if (!definitions.is_open() || !provinces.loadFromFile("map/provinces.bmp")) return;

			auto pixels(std::move(utl::imageToPixelArray(provinces)));
			std::vector<sf::Vector2s> unassignedBorderTriangles;
			std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>> provinceContourPoints;

			regions.reserve(maxProvinces);

			while (!definitions.eof() && regions.size() != maxProvinces)
			{
				std::string buffer;

				std::getline(definitions, buffer, '\n');
				std::stringstream line(buffer);
				sf::Color provinceColor;

				std::getline(line, buffer, ';');	
				unsigned short provID(std::stoi(buffer));

				std::cout << provID << std::endl; //

				std::getline(line, buffer, ';');
				provinceColor.r = std::stoi(buffer);
				std::getline(line, buffer, ';');
				provinceColor.g = std::stoi(buffer);
				std::getline(line, buffer, ';');
				provinceColor.b = std::stoi(buffer);

				if (provinceColor.r == 0 && provinceColor.g == 0 && provinceColor.b == 0) break;

				regions[provID].traceShape(pixels, provinceColor, unassignedBorderTriangles, provinceContourPoints[provID]);
			}

			regions[unsigned short(-1)].traceShape(pixels, sf::Color::White, unassignedBorderTriangles, provinceContourPoints[unsigned short(-1)]);
			
			assignBorderTriangles(unassignedBorderTriangles, provinceContourPoints);

			createProvinceCache();
		}
		else loadProvinceCache();
	}

	void Map::updateRegionVisuals(const sf::Vector2s newResolution)
	{
		resolution = newResolution;

		viewBounds = sf::FloatRect(0, 0, resolution.x, resolution.y);
		mapView.reset(sf::FloatRect(1.0f, 1.0f, resolution.x, resolution.y));

		border.second = sf::Vector2f(1.0f / resolution.x, 1.0f / resolution.y);
		border.first.setParameter("step", border.second);

		land.create(resolution.x, resolution.y);
		land.setSmooth(true);
		sea.create(resolution.x, resolution.y);
		sea.setSmooth(true);
	}

	void Map::loadResources()
	{
		{
			const float stripeWidth(0.4), tileSize(64);

			sf::RenderTexture stripeTarget;
			stripeTarget.create(tileSize, tileSize);
			stripeTarget.clear(sf::Color(0, 0, 0, 0));

			sf::VertexArray stripe(sf::PrimitiveType::Quads);
			stripe.append(sf::Vertex(sf::Vector2f(-(stripeWidth) / 2.0f - tileSize * 1.5f, (stripeWidth) / 2.0f - tileSize * 1.5f), sf::Color(255, 255, 255, 255)));
			stripe.append(sf::Vertex(sf::Vector2f((stripeWidth) / 2.0f - tileSize * 1.5f, -(stripeWidth) / 2.0f - tileSize * 1.5f), sf::Color(255, 255, 255, 255)));
			stripe.append(sf::Vertex(sf::Vector2f((stripeWidth) / 2.0f + tileSize * 1.5f, -(stripeWidth) / 2.0f + tileSize * 1.5f), sf::Color(255, 255, 255, 255)));
			stripe.append(sf::Vertex(sf::Vector2f(-(stripeWidth) / 2.0f + tileSize * 1.5f, (stripeWidth) / 2.0f + tileSize * 1.5f), sf::Color(255, 255, 255, 255)));

			sf::Transform translate;
			translate.translate(-2 * tileSize, 0);
			while (translate.transformRect(stripe.getBounds()).left <= tileSize)
			{
				stripeTarget.draw(stripe, translate);
				translate.translate(tileSize / 15.0f, 0);
			}

			stripeTarget.display();

			stripes = stripeTarget.getTexture();
			stripes.setRepeated(true);
		}

		mapTile.loadFromFile("map/resources/mapTile.png");

		border.first.loadFromMemory(borderShaderCode, sf::Shader::Fragment);
		border.first.setParameter("texture", sf::Shader::CurrentTexture);
		border.first.setParameter("color", sf::Color(5, 10, 15, 255));
		border.first.setParameter("step", border.second);

		landProvinces.setPrimitiveType(sf::PrimitiveType::Triangles);
		provinceStripes.setPrimitiveType(sf::PrimitiveType::Triangles);
		seaProvinces.setPrimitiveType(sf::PrimitiveType::Lines);

		stripesBuffer[0].setPrimitiveType(sf::PrimitiveType::Triangles);
		landBuffer[0].setPrimitiveType(sf::PrimitiveType::Triangles);
		seaBuffer[0].setPrimitiveType(sf::PrimitiveType::Lines);

		stripesBuffer[1].setPrimitiveType(sf::PrimitiveType::Triangles);
		landBuffer[1].setPrimitiveType(sf::PrimitiveType::Triangles);
		seaBuffer[1].setPrimitiveType(sf::PrimitiveType::Lines);

		oceanGradient.clear();
		oceanGradient.setPrimitiveType(sf::PrimitiveType::Quads);
		oceanGradient.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(125, 130, 165, 255), sf::Vector2f(0.0f, 0.0f)));
		oceanGradient.append(sf::Vertex(sf::Vector2f(mapSize.x, 0.0f), sf::Color(125, 130, 165, 255), sf::Vector2f(mapSize.x, 0.0f)));
		oceanGradient.append(sf::Vertex(sf::Vector2f(mapSize.x, mapSize.y), sf::Color(135, 195, 205, 255), sf::Vector2f(mapSize.x, mapSize.y)));
		oceanGradient.append(sf::Vertex(sf::Vector2f(0.0f, mapSize.y), sf::Color(135, 195, 205, 255), sf::Vector2f(0.0f, mapSize.y)));
	}

	void Map::draw(sf::RenderTarget& target, sf::RenderStates states)const
	{
		sea.setView(mapView);
		sea.clear(sf::Color(0, 0, 0, 0));

		land.setView(mapView);
		land.clear(sf::Color(0, 0, 0, 0));

		sea.draw(oceanGradient, &mapTile);
		land.draw(landBuffer[drawableBufferSet], &mapTile);
		land.draw(stripesBuffer[drawableBufferSet], &stripes);

		if (viewBounds.left < 0)
		{
			sf::RenderStates states;
			states.transform.translate((-1) * mapSize.x, 0);
			states.texture = &mapTile;

			sea.draw(oceanGradient, states);
			land.draw(landBuffer[drawableBufferSet], states);
			states.texture = &stripes;
			land.draw(stripesBuffer[drawableBufferSet], states);
		}

		sea.display();
		land.display();

		sf::Sprite bufferSprite(sea.getTexture());
		target.draw(bufferSprite, &border.first);
		bufferSprite.setTexture(land.getTexture());
		target.draw(bufferSprite, &border.first);
	}

	std::unique_ptr<gui::Interactive> Map::copy() const
	{
		return std::unique_ptr<Map>(new Map(*this));
	}

	std::unique_ptr<gui::Interactive> Map::move()
	{
		return std::unique_ptr<Map>(new Map(std::move(*this)));
	}

	const bool Map::input(const sf::Event& event)
	{
		switch (event.type)
		{
		case sf::Event::MouseMoved:
			if (event.mouseMove.x == resolution.x || event.mouseMove.x == resolution.x - 1)
				scrollAmount.x = scrollStep;
			else if (event.mouseMove.x == 0 || event.mouseMove.x == 1)
				scrollAmount.x = -scrollStep;
			else scrollAmount.x = 0;

			if (event.mouseMove.y == resolution.y || event.mouseMove.y == resolution.y - 1)
				scrollAmount.y = scrollStep;
			else if (event.mouseMove.y == 0 || event.mouseMove.y == 1)
				scrollAmount.y = -scrollStep;
			else scrollAmount.y = 0;
		case sf::Event::MouseWheelMoved:
			zoom(sf::Vector2s(event.mouseWheel.x, event.mouseWheel.y), event.mouseWheel.delta > 0.0f ? zoomFactor : 1.0f / zoomFactor);
		case sf::Event::MouseButtonReleased:
			clickCheck(sf::Vector2s(event.mouseButton.x, event.mouseButton.y));
		default:
			return false;
		}
		return true;
	}

	const sf::FloatRect Map::getGlobalBounds() const
	{
		return viewBounds;
	}

	const sf::Vector2f& Map::getPosition() const
	{
		return sf::Vector2f(viewBounds.left, viewBounds.top);
	}

	Map& Map::setPosition(const float x, const float y)
	{
		scroll(sf::Vector2f(x - viewBounds.left, y - viewBounds.top));
	}

	const std::vector<std::vector<sf::Vector2s>> Map::tesselatePolygonArray(const std::vector<std::vector<sf::Vector2s>>& polygons)
	{
		std::vector<std::vector<sf::Vector2s>> returnValue;
		for (auto it = polygons.begin(), end = polygons.end(); it != end; ++it)
		{
			returnValue.emplace_back();
			utl::tesselateShape(utl::simplifyShape(*it), returnValue.back());
		}
		return returnValue;
	}
	
	std::pair<std::pair<std::vector<sf::Vector2s>::const_iterator, bool>, sf::Vector2s> findCommonPoints(const std::vector<sf::Vector2s>& polygon, const std::vector<sf::Vector2s>& triangle)
	{
		std::pair<std::pair<std::vector<sf::Vector2s>::const_iterator, bool>, sf::Vector2s> returnValue(std::make_pair(polygon.end(), false), sf::Vector2s(-1, -1));
		if (triangle.size() != 3) return returnValue;

		for (auto it(polygon.begin()), end(polygon.end()); it != end; ++it)
			if (std::find(triangle.begin(), triangle.end(), *it) != triangle.end())
			{
				if (returnValue.first.first == polygon.end())
					returnValue.first.first = it;
				else if (returnValue.second.x == -1)
				{
					returnValue.second = *std::find_if(triangle.begin(), triangle.end(), [&](const sf::Vector2s& element)
					{
						return element != *it && element != *returnValue.first.first;
					});
					returnValue.first.first = it;
				}
				else
				{
					returnValue.first.second = true;
					return returnValue;
				}
			}
			else
			{
				if (returnValue.second.x != -1) return returnValue;
				returnValue = std::make_pair(std::make_pair(polygon.end(), false), sf::Vector2s(-1, -1));
			}

		return std::make_pair(std::make_pair(polygon.end(), false), sf::Vector2s(-1, -1));
	}

	void Map::assignBorderTriangles(std::vector<sf::Vector2s>& unassignedTriangles, std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>>& provinceContours)
	{
		std::cout << std::endl; //
		for (auto it(provinceContours.begin()), end(provinceContours.end()); it != end; ++it)
		{
			std::cout << it->first << std::endl; //

			const sf::Color color;
			regions.at(it->first).indexBegin = (bEnd::Region::regions[it->first].sea ? seaProvinces : landProvinces).getVertexCount();

			for (auto it1(it->second.begin()), end1(it->second.end()); it1 != end1; ++it1)
			{
				for (size_t i(0), end3(unassignedTriangles.size()); i < end3; i += 3)
				{
					if (unassignedTriangles.at(i).x == -1) continue;

					std::pair<std::pair<std::vector<sf::Vector2s>::const_iterator, bool>, sf::Vector2s> commonPoints;

					if ((commonPoints = findCommonPoints(*it1, std::vector<sf::Vector2s> {unassignedTriangles.at(i), 
						unassignedTriangles.at(i + 1), unassignedTriangles.at(i + 2)})).first.first != it1->end())
					{
						unassignedTriangles.at(i).x = -1;
						if (commonPoints.first.second) it1->erase(commonPoints.first.first);
						else if(std::find(it1->begin(), it1->end(), commonPoints.second) == it1->end()) it1->emplace(commonPoints.first.first, commonPoints.second);
					}
				}
				
				if (bEnd::Region::regions[it->first].sea)
				{
					const std::vector<sf::Vector2s> finalProvince(std::move(utl::simplifyShape(*it1)));

					sf::Vector2f vertex(finalProvince.front().x, finalProvince.front().y);
					seaProvinces.append(sf::Vertex(vertex, color, vertex));
					for (auto it2(finalProvince.begin() + 1), end2(finalProvince.end() - 1); it2 != end2; ++it2)
					{
						vertex.x = (it2->x);
						vertex.y = (it2->y);
						seaProvinces.append(sf::Vertex(vertex, color, vertex));
						seaProvinces.append(seaProvinces[seaProvinces.getVertexCount() - 1]);
					}
					vertex.x = finalProvince.back().x;
					vertex.y = finalProvince.back().y;
					seaProvinces.append(sf::Vertex(vertex, color, vertex));
				}
				else
				{
					std::vector<sf::Vector2s> finalProvince;
					utl::tesselateShape(utl::simplifyShape(*it1), finalProvince);

					for (auto it2(finalProvince.begin()), end2(finalProvince.end()); it2 != end2; ++it2)
					{
						const sf::Vector2f vertex(it2->x, it2->y);
						landProvinces.append(sf::Vertex(vertex, color, vertex));
					}
				}
			}

			regions.at(it->first).indexEnd = (bEnd::Region::regions[it->first].sea ? seaProvinces : landProvinces).getVertexCount() - 1;
		}
	}

	void Map::createProvinceCache()
	{
		std::ofstream cache("map/cache/provinces.bin", std::ios::out | std::ios::binary);
		for (auto it(regions.begin()), end(regions.end()); it != end; ++it)
		{
			const bool sea(bEnd::Region::exists(it->first) ? (bEnd::Region::regions.at(it->first).sea ? true : false) : false);

			cache.write((char*)&it->first, sizeof(unsigned short));
			cache.write((char*)&sea, sizeof(bool));

			const unsigned int bytes((regions.at(it->first).indexEnd - regions.at(it->first).indexBegin + 1) * 2 * sizeof(unsigned short));
			cache.write((char*)&bytes, sizeof(unsigned int));

			for (auto i(it->second.indexBegin), end(it->second.indexEnd); i <= end; i++)
			{
				auto source(sea ? seaProvinces : landProvinces);

				cache.write((char*)&source[i].position.x, sizeof(unsigned short));
				cache.write((char*)&source[i].position.y, sizeof(unsigned short));
			}
		}
	}

	void Map::loadProvinceCache()
	{
		std::ifstream cache("map/cache/provinces.bin", std::ios::in | std::ios::binary);

		while (!cache.eof())
		{
			unsigned short provID;
			cache.read((char*)&provID, sizeof(unsigned short));

			cache.read((char*)&bEnd::Region::regions[provID].sea, sizeof(bool));
			auto& target(bEnd::Region::regions.at(provID).sea ? seaProvinces : landProvinces);

			unsigned int bytes;
			cache.read((char*)&bytes, sizeof(unsigned int));

			regions[provID].indexBegin = target.getVertexCount();
			for (bytes; bytes > 0; bytes -= 4)
			{
				sf::Vector2s vertex;
				cache.read((char*)&vertex.x, sizeof(unsigned short));
				cache.read((char*)&vertex.y, sizeof(unsigned short));
				target.append(sf::Vertex(sf::Vector2f(vertex.x, vertex.y), sf::Color(), sf::Vector2f(vertex.x, vertex.y)));
			}
			regions.at(provID).indexEnd = target.getVertexCount() - 1;
		}

		provinceStripes = landProvinces;
	}

	void Map::scroll()
	{
		if (gui::Duration(gui::Internals::timeSinceStart() - timeOfLastScroll) > gui::Duration(1.0f / 30))
		{
			mapView.move(scrollAmount.x, scrollAmount.y);
			viewBounds.left += scrollAmount.x;
			viewBounds.top += scrollAmount.y;
			timeOfLastScroll = gui::Internals::timeSinceStart();
		}
	}

	void Map::zoom(const sf::Vector2s& targetPoint, const float factor)
	{

	}

	void Map::clickCheck(const sf::Vector2s& point)
	{

	}

	void Map::Region::traceShape(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
		std::vector<sf::Vector2s>& borderTrianglesTarget, std::vector<std::vector<sf::Vector2s>>& contourPointsTarget)
	{
		auto points(std::move(utl::marchingSquares(pixels, colorCode)));

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			if (it->empty()) continue;

			*it = std::move(utl::cullBorderTriangles(pixels, colorCode, *it, borderTrianglesTarget));

			contourPointsTarget.emplace_back();
			for (auto it1 = it->begin(), end1 = it->end(); it1 != end1; ++it1)
				contourPointsTarget.back().push_back(*it1);
		}
	}
}