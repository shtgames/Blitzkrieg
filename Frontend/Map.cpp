#include "Map.h"

#include "../Backend/FileProcessor.h"
#include "../Backend/Region.h"
#include "Nation.h"
#include "utilities.h"

#include <fstream>
#include <sstream>
#include <set>

namespace fEnd
{
	const std::string borderShaderCode = 
		"uniform vec2 step;\
		uniform sampler2D texture;\
		uniform vec4 color;\
		\
		vec4 borderEffect()\
		{\
			float outlineIntensity = 8.0f * texture2D(texture, gl_TexCoord[0].xy).a;\
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

	const std::string stripeShaderCode =
		"uniform sampler2D stripes, background;\
		uniform float zoomFactor;\
		\
		void main()\
		{\
			gl_FragColor.rgb = (gl_Color * texture2D(background, gl_TexCoord[0].xy / zoomFactor)).rgb;\
			gl_FragColor.a = texture2D(stripes, gl_TexCoord[0].xy).a;\
		}";

	std::unordered_map<unsigned short, Map::Region> Map::regions;

	sf::Vector2s Map::mapSize;
	sf::RenderTexture Map::land, Map::sea;

	sf::VertexArray Map::oceanGradient, Map::provinceStripes, Map::landProvinces, Map::seaProvinces;
	sf::VertexArray Map::stripesBuffer[2], Map::landBuffer[2], Map::seaBuffer[2];
	std::atomic<bool> Map::drawableBufferSet, Map::vertexArraysVisibilityNeedsUpdate = false, Map::updateThreadLaunched = false;

	sf::Texture Map::mapTile, Map::stripes;
	std::pair<sf::Shader, sf::Vector2f> Map::border;
	sf::Shader Map::stripesShader;
	Camera Map::camera;

	void Map::addRegionNeedingColorUpdate(const unsigned short regionID)
	{
		colorUpdateQueueLock.lock();
		regionsNeedingColorUpdate.emplace(regionID);
		colorUpdateQueueLock.unlock();
	}

	void Map::loadRegions()
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

		if (!std::ifstream("map/cache/provinces.bin").good())
		{
			std::ifstream definitions("map/definition.csv");
			sf::Image provinces;

			if (!definitions.is_open() || !provinces.loadFromFile("map/provinces.bmp")) return;

			auto pixels(std::move(utl::imageToPixelArray(provinces)));
			std::vector<sf::Vector2s> unassignedBorderTriangles;
			std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>> provinceContourPoints;

			regions.reserve(maxProvinces);

			while (!definitions.eof() && regions.size() != maxProvinces - 1)
			{
				std::string buffer;

				std::getline(definitions, buffer, '\n');
				std::stringstream line(buffer);
				sf::Color provinceColor;

				std::getline(line, buffer, ';');	
				unsigned short provID(std::stoi(buffer));
				
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

	void Map::updateRegionVisuals(const sf::Vector2s& newResolution)
	{
		border.second = sf::Vector2f(1.0f / newResolution.x, 1.0f / newResolution.y);
		border.first.setParameter("step", border.second);

		land.create(newResolution.x, newResolution.y);
		sea.create(newResolution.x, newResolution.y);

		const float maxZoomFactor(newResolution.y / (mapSize.y * Camera::upperZoomLimitAsMapSizeFraction));

		stripesShader.loadFromMemory(stripeShaderCode, sf::Shader::Fragment);
		stripesShader.setParameter("stripes", stripes);
		stripesShader.setParameter("background", sf::Shader::CurrentTexture);
		stripesShader.setParameter("zoomFactor", maxZoomFactor);

		for (size_t it(0), end(provinceStripes.getVertexCount()); it != end; ++it)
		{
			provinceStripes[it].color.a = 0;//
			provinceStripes[it].texCoords.x = provinceStripes[it].position.x * maxZoomFactor;
			provinceStripes[it].texCoords.y = provinceStripes[it].position.y * maxZoomFactor;
		}

		createStripesTexture(stripes, 64 * maxZoomFactor, 0.5f * maxZoomFactor);
		stripes.setRepeated(true);

		camera.setPosition(0, 0)
			.setSize(newResolution)
			.setScreenResolution(newResolution)
			.setMapSize(mapSize);
	}

	void Map::createStripesTexture(sf::Texture& targetTexture, const float size, const float stripeWidth)
	{
		sf::RenderTexture buffer;

		buffer.create(size, size);
		buffer.clear(sf::Color(0, 0, 0, 0));

		sf::VertexArray stripe(sf::PrimitiveType::Quads);
		stripe.append(sf::Vertex(sf::Vector2f(-(stripeWidth) / 2.0f - size * 1.5f, (stripeWidth) / 2.0f - size * 1.5f), sf::Color(255, 255, 255, 255)));
		stripe.append(sf::Vertex(sf::Vector2f((stripeWidth) / 2.0f - size * 1.5f, -(stripeWidth) / 2.0f - size * 1.5f), sf::Color(255, 255, 255, 255)));
		stripe.append(sf::Vertex(sf::Vector2f((stripeWidth) / 2.0f + size * 1.5f, -(stripeWidth) / 2.0f + size * 1.5f), sf::Color(255, 255, 255, 255)));
		stripe.append(sf::Vertex(sf::Vector2f(-(stripeWidth) / 2.0f + size * 1.5f, (stripeWidth) / 2.0f + size * 1.5f), sf::Color(255, 255, 255, 255)));

		sf::Transform translate;
		translate.translate(-2 * size, 0);
		while (translate.transformRect(stripe.getBounds()).left <= size)
		{
			buffer.draw(stripe, translate);
			translate.translate(size / 15.0f, 0);
		}

		buffer.display();

		targetTexture = buffer.getTexture();
	}

	void Map::loadResources()
	{
		land.setSmooth(true);
		sea.setSmooth(true);

		mapTile.loadFromFile("map/resources/mapTile.png");
		mapTile.setRepeated(true);
		mapTile.setSmooth(true);

		border.first.loadFromMemory(borderShaderCode, sf::Shader::Fragment);
		border.first.setParameter("texture", sf::Shader::CurrentTexture);
		border.first.setParameter("color", sf::Color(5, 10, 15, 255));
		border.first.setParameter("step", border.second);
		
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

		if (!updateThreadLaunched)
		{
			std::thread updateThread(updateVertexArrays);
			updateThread.detach();

			updateThreadLaunched = true;
		}
	}

	void Map::draw(sf::RenderTarget& target, sf::RenderStates states)const
	{
		sea.draw(camera, states);
		sea.clear(sf::Color(0, 0, 0, 0));

		land.draw(camera, states);
		land.clear(sf::Color(0, 0, 0, 0));

		sea.draw(oceanGradient, &mapTile);
		land.draw(landBuffer[drawableBufferSet], &mapTile);

		states.shader = &stripesShader;
		states.texture = &mapTile;
		land.draw(stripesBuffer[drawableBufferSet], states);

		if (camera.getPosition().x < 0)
		{
			states.shader = nullptr;
			states.transform.translate(-mapSize.x, 0);
			states.texture = &mapTile;

			sea.draw(oceanGradient, states);
			land.draw(landBuffer[drawableBufferSet], states);
			states.texture = &stripes;
			land.draw(stripesBuffer[drawableBufferSet], states);
		}

		sea.display();
		land.display();

		border.first.setParameter("step", border.second / camera.getTotalZoom());
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
		case sf::Event::MouseWheelMoved:
			camera.input(event);
			break;
		case sf::Event::MouseButtonReleased:
			clickCheck(sf::Vector2s(event.mouseButton.x, event.mouseButton.y));
			break;
		default:
			return false;
		}
		return true;
	}

	const sf::FloatRect Map::getGlobalBounds() const
	{
		return camera.getGlobalBounds();
	}

	const sf::Vector2f& Map::getPosition() const
	{
		return camera.getPosition();
	}

	Map& Map::setPosition(const float x, const float y)
	{
		camera.setPosition(x, y);
		return *this;
	}

	std::pair<std::pair<std::vector<sf::Vector2s>::const_iterator, bool>, sf::Vector2s> findCommonPoints(const std::vector<sf::Vector2s>& polygon, const std::vector<sf::Vector2s>& triangle)
	{
		auto returnValue(std::make_pair(std::make_pair(polygon.end(), false), sf::Vector2s(-1, -1)));
		if (triangle.size() != 3) return returnValue;

		bool firstPass = true;
		for (auto begin(polygon.begin()), end(polygon.end()), it(begin); it != begin + 2 || firstPass; ++it)
		{
			if (it == end)
			{
				firstPass = false;
				it = begin;
			}
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
		}

		return std::make_pair(std::make_pair(polygon.end(), false), sf::Vector2s(-1, -1));
	}

	void Map::assignBorderTriangles(std::vector<sf::Vector2s>& unassignedTriangles, std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>>& provinceContours)
	{
		for (auto it(provinceContours.begin()), end(provinceContours.end()); it != end; ++it)
		{
			const sf::Color color;
			regions.at(it->first).indexBegin = (bEnd::Region::regions[it->first].sea ? seaProvinces : landProvinces).getVertexCount();

			for (auto it1(it->second.begin()), end1(it->second.end()); it1 != end1; ++it1)
			{
				bool firstPass = true;
				while (true)
				{
					for (size_t i(0), end3(unassignedTriangles.size()); i < end3; i += 3)
					{
						if (unassignedTriangles.at(i).x == -1) continue;

						auto commonPoints(findCommonPoints(*it1, std::vector<sf::Vector2s> {unassignedTriangles.at(i),
							unassignedTriangles.at(i + 1), unassignedTriangles.at(i + 2)}));

						if (commonPoints.first.first != it1->end())
						{
							unassignedTriangles.at(i).x = -1;
							if (commonPoints.first.second) it1->erase(commonPoints.first.first);
							else if (std::find(it1->begin(), it1->end(), commonPoints.second) == it1->end()) it1->emplace(commonPoints.first.first, commonPoints.second);
						}
					}
					if (firstPass) firstPass = false;
					else break;
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

			regions.at(it->first).indexEnd = (bEnd::Region::regions[it->first].sea ? seaProvinces : landProvinces).getVertexCount();
		}
		provinceStripes = landProvinces;
	}
	
	void Map::createProvinceCache()
	{
		std::ofstream cache("map/cache/provinces.bin", std::ios::out | std::ios::binary);
		for (auto it(regions.begin()), end(regions.end()); it != end; ++it)
		{
			const bool sea(bEnd::Region::exists(it->first) ? (bEnd::Region::get(it->first).sea ? true : false) : false);
			auto source(sea ? seaProvinces : landProvinces);

			cache.write((char*)&it->first, sizeof(unsigned short));

			const unsigned short bytes((regions.at(it->first).indexEnd - regions.at(it->first).indexBegin) * 2 * 2);
			cache.write((char*)&bytes, sizeof(bytes));

			for (auto i(it->second.indexBegin), end(it->second.indexEnd); i < end; i++)
			{
				sf::Vector2s vertex(source[i].position);
				cache.write((char*)&vertex.x, sizeof(short));
				cache.write((char*)&vertex.y, sizeof(short));
			}
		}
	}

	void Map::loadProvinceCache()
	{
		std::ifstream cache("map/cache/provinces.bin", std::ios::in | std::ios::binary);

		const sf::Color color(200, 200, 200);
		while (!cache.eof())
		{
			unsigned short provID(0);
			cache.read((char*)&provID, sizeof(unsigned short));

			auto& target(bEnd::Region::regions[provID].sea ? seaProvinces : landProvinces);

			unsigned short bytes(0);
			cache.read((char*)&bytes, sizeof(unsigned short));


			regions[provID].indexBegin = target.getVertexCount();
			for (bytes; bytes > 0; bytes -= 4)
			{
				sf::Vector2s vertex(0, 0);
				cache.read((char*)&vertex.x, sizeof(short));
				cache.read((char*)&vertex.y, sizeof(short));
				target.append(sf::Vertex(sf::Vector2f(vertex), color, sf::Vector2f(vertex)));
			}
			regions.at(provID).indexEnd = target.getVertexCount();
		}

		provinceStripes = landProvinces;
	}

	void Map::clickCheck(const sf::Vector2s& point)
	{
		//
	}

	void Map::updateVertexArrays()
	{
		gui::TimePoint timeOfLastUpdate(gui::Internals::timeSinceStart());

		while (1)
			if (gui::Duration(gui::Internals::timeSinceStart() - timeOfLastUpdate) < gui::Duration(0.015f)) continue;			
			else if (camera.hasChanged || vertexArraysVisibilityNeedsUpdate)
			{
				timeOfLastUpdate = gui::Internals::timeSinceStart();

				const bool targetSet(!drawableBufferSet);

				const sf::FloatRect bounds(camera.getGlobalBounds());

				landBuffer[targetSet].clear();
				stripesBuffer[targetSet].clear();
				seaBuffer[targetSet].clear();

				for (size_t i(0), end(landProvinces.getVertexCount()); i != end; i += 3)
					if (bounds.contains(landProvinces[i].position) ||
						bounds.contains(landProvinces[i + 1].position) ||
						bounds.contains(landProvinces[i + 2].position))
					{
						landBuffer[targetSet].append(landProvinces[i]);
						landBuffer[targetSet].append(landProvinces[i + 1]);
						landBuffer[targetSet].append(landProvinces[i + 2]);

						if (provinceStripes[i].color.a != 0)
						{
							stripesBuffer[targetSet].append(provinceStripes[i]);
							stripesBuffer[targetSet].append(provinceStripes[i + 1]);
							stripesBuffer[targetSet].append(provinceStripes[i + 2]);
						}
					}
								
				for (size_t i(0), end(seaProvinces.getVertexCount()); i != end; i += 2)
					if (bounds.contains(seaProvinces[i].position) ||
						bounds.contains(seaProvinces[i + 1].position))
					{
						seaBuffer[targetSet].append(seaProvinces[i]);
						seaBuffer[targetSet].append(seaProvinces[i + 1]);
					}

				if (bounds.left < 0)
				{
					for (size_t i(0), end(landProvinces.getVertexCount()); i != end; i += 3)
						if (bounds.contains(landProvinces[i].position.x - mapSize.x, landProvinces[i].position.y) ||
							bounds.contains(landProvinces[i + 1].position.x - mapSize.x, landProvinces[i + 1].position.y) ||
							bounds.contains(landProvinces[i + 2].position.x - mapSize.x, landProvinces[i + 2].position.y))
						{
							const auto* vertex(&landProvinces[i]);
							landBuffer[targetSet].append(sf::Vertex(sf::Vector2f(vertex->position.x - mapSize.x, vertex->position.y),
								vertex->color, vertex->texCoords));
							landBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &landProvinces[i + 1])->position.x - mapSize.x, vertex->position.y),
								vertex->color, vertex->texCoords));
							landBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &landProvinces[i + 2])->position.x - mapSize.x, vertex->position.y),
								vertex->color, vertex->texCoords));

							if (provinceStripes[i].color.a != 0)
							{
								vertex = &provinceStripes[i];
								stripesBuffer[targetSet].append(sf::Vertex(sf::Vector2f(vertex->position.x - mapSize.x, vertex->position.y),
									vertex->color, vertex->texCoords));
								stripesBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &provinceStripes[i + 1])->position.x - mapSize.x, vertex->position.y),
									vertex->color, vertex->texCoords));
								stripesBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &provinceStripes[i + 2])->position.x - mapSize.x, vertex->position.y),
									vertex->color, vertex->texCoords));
							}
						}

					for (size_t i(0), end(seaProvinces.getVertexCount()); i != end; i += 2)
						if (bounds.contains(seaProvinces[i].position.x - mapSize.x, seaProvinces[i].position.y) ||
							bounds.contains(seaProvinces[i + 1].position.x - mapSize.x, seaProvinces[i + 1].position.y))
						{
							const auto* vertex(&seaProvinces[i]);
							seaBuffer[targetSet].append(sf::Vertex(sf::Vector2f(vertex->position.x - mapSize.x, vertex->position.y),
								vertex->color, vertex->texCoords));
							seaBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &seaProvinces[i + 1])->position.x - mapSize.x, vertex->position.y),
								vertex->color, vertex->texCoords));
						}
				}

				drawableBufferSet = !drawableBufferSet;
				camera.hasChanged = false;
				vertexArraysVisibilityNeedsUpdate = false;
			}
			else
			{
				colorUpdateQueueLock.lock();
				while (regionsNeedingColorUpdate.size() != 0)
				{
					if (bEnd::Region::get(regionsNeedingColorUpdate.front()).sea)
					{
						regionsNeedingColorUpdate.pop();
						continue;
					}
					for (auto it(regions[regionsNeedingColorUpdate.front()].indexBegin), end(regions[regionsNeedingColorUpdate.front()].indexEnd); it != end; ++it)
					{
						landProvinces[it].color = Nation::get(bEnd::Region::get(regionsNeedingColorUpdate.front()).owner).getColor();
						provinceStripes[it].color = Nation::get(bEnd::Region::get(regionsNeedingColorUpdate.front()).controller).getColor();
					}
					regionsNeedingColorUpdate.pop();
				}
				colorUpdateQueueLock.unlock();
			}
	}

	void Map::Region::traceShape(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
		std::vector<sf::Vector2s>& borderTrianglesTarget, std::vector<std::vector<sf::Vector2s>>& contourPointsTarget)
	{
		auto points(std::move(utl::marchingSquares(pixels, colorCode)));

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			if (it->empty()) continue;
			
			utl::cullBorderTriangles(pixels, colorCode, *it, borderTrianglesTarget);

			contourPointsTarget.emplace_back();
			for (auto it1 = it->begin(), end1 = it->end(); it1 != end1; ++it1)
				contourPointsTarget.back().push_back(*it1);
		}
	}
}