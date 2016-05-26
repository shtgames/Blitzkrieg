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

	gui::FadeAnimation Map::animation;

	std::unordered_map<unsigned short, Map::Region> Map::regions;

	sf::Vector2s Map::mapSize;
	sf::RenderTexture Map::land;

	sf::VertexArray Map::oceanGradient, Map::provinceStripes, Map::landProvinces, Map::seaProvinces;
	sf::VertexArray Map::stripesBuffer[2], Map::landBuffer[2], Map::seaBuffer[2];
	volatile std::atomic<bool> Map::drawableBufferSet, Map::vertexArraysVisibilityNeedsUpdate = false, Map::updateThreadLaunched = false;
	std::queue<unsigned short> Map::regionsNeedingColorUpdate;
	std::mutex Map::colorUpdateQueueLock;

	sf::Texture Map::mapTile, Map::stripes;
	std::pair<sf::Shader, sf::Vector2f> Map::border;
	sf::Shader Map::stripesShader;
	Camera Map::camera;
	std::unique_ptr<unsigned short> Map::target;
	
	sf::VertexArray mapBuffer;

	void Map::addRegionNeedingColorUpdate(const unsigned short regionID)
	{
		colorUpdateQueueLock.lock();
		regionsNeedingColorUpdate.emplace(regionID);
		colorUpdateQueueLock.unlock();
	}

	void Map::updateAllRegionColors()
	{
		for (auto& it : regions)
			if (bEnd::Region::get(it.first).sea) continue;
			else for (auto i(it.second.indexBegin), end(it.second.indexEnd); i != end; ++i)
			{
				landProvinces[i].color = Nation::get(bEnd::Region::get(it.first).controller).getColor(); 
				if (bEnd::Region::get(it.first).controller != bEnd::Region::get(it.first).owner)
					provinceStripes[i].color = Nation::get(bEnd::Region::get(it.first).owner).getColor();
				else provinceStripes[i].color.a = 0;
			}

		vertexArraysVisibilityNeedsUpdate = true;
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
				const unsigned short provID(std::stoi(buffer));
				
				std::getline(line, buffer, ';');
				provinceColor.r = std::stoi(buffer);
				std::getline(line, buffer, ';');
				provinceColor.g = std::stoi(buffer);
				std::getline(line, buffer, ';');
				provinceColor.b = std::stoi(buffer);

				if (provinceColor.r == 0 && provinceColor.g == 0 && provinceColor.b == 0) break;

				regions[provID].traceShape(pixels, provinceColor, unassignedBorderTriangles, provinceContourPoints[provID]);
			}

			bEnd::Region::regions[-1].sea = true;
			regions[-1].traceShape(pixels, sf::Color::White, unassignedBorderTriangles, provinceContourPoints[-1]);
			
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

		const float maxZoomFactor(newResolution.y / (mapSize.y * Camera::upperZoomLimitAsMapSizeFraction));

		stripesShader.loadFromMemory(stripeShaderCode, sf::Shader::Fragment);
		stripesShader.setParameter("stripes", stripes);
		stripesShader.setParameter("background", sf::Shader::CurrentTexture);
		stripesShader.setParameter("zoomFactor", maxZoomFactor);

		for (size_t it(0), end(provinceStripes.getVertexCount()); it != end; ++it)
		{
			provinceStripes[it].color.a = 0;
			provinceStripes[it].texCoords.x = provinceStripes[it].position.x * maxZoomFactor;
			provinceStripes[it].texCoords.y = provinceStripes[it].position.y * maxZoomFactor;
		}

		createStripesTexture(stripes, 64 * maxZoomFactor, 0.5f * maxZoomFactor);
		stripes.setRepeated(true);

		camera.setPosition(0, 0)
			.setSize(newResolution)
			.setScreenResolution(newResolution)
			.setMapSize(mapSize);

		mapBuffer.clear();
		mapBuffer.setPrimitiveType(sf::PrimitiveType::Quads);
		mapBuffer.append(sf::Vertex(sf::Vector2f(0, 0), sf::Vector2f(0, 0)));
		mapBuffer.append(sf::Vertex(sf::Vector2f(newResolution.x, 0), sf::Vector2f(newResolution.x, 0)));
		mapBuffer.append(sf::Vertex(sf::Vector2f(newResolution.x, newResolution.y), sf::Vector2f(newResolution.x, newResolution.y)));
		mapBuffer.append(sf::Vertex(sf::Vector2f(0, newResolution.y), sf::Vector2f(0, newResolution.y)));
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

		animation.setDuration(1.0f);
		animation.setFadeDirection(0);
	}

	void Map::launchRegionUpdateThread()
	{
		if (updateThreadLaunched) return;

		updateThreadLaunched = true;

		std::thread updateThread(updateVertexArrays);
		updateThread.detach();
	}

	void Map::stopRegionUpdateThread()
	{
		updateThreadLaunched = false;
	}

	const sf::FloatRect Map::getViewBounds()
	{
		return camera.getGlobalBounds();
	}

	void Map::setViewPosition(const float x, const float y)
	{
		camera.setPosition(x, y);
	}

	void Map::draw(sf::RenderTarget& target, sf::RenderStates states)const
	{
		target.draw(camera, states);

		land.draw(camera, states);
		land.clear(sf::Color(0, 0, 0, 0));

		target.draw(oceanGradient, &mapTile);
		if (camera.getPosition().x < 0)
		{
			states.transform.translate(-mapSize.x, 0);
			states.texture = &mapTile;

			target.draw(oceanGradient, states);

			states.transform.translate(mapSize.x, 0);
		}

		states.shader = &stripesShader;
		states.texture = &mapTile;
		land.draw(stripesBuffer[drawableBufferSet], states);

		target.draw(animation);
		if (animation.getFadeAmount() != 0.0f)
			target.draw(seaBuffer[drawableBufferSet], &animation.getShaderNonTextured());

		land.draw(landBuffer[drawableBufferSet], &mapTile);

		land.display();

		border.first.setParameter("step", border.second / camera.getTotalZoom());

		states.shader = &border.first;
		states.texture = &land.getTexture();
		target.setView(target.getDefaultView());
		target.draw(mapBuffer, states);
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
			return camera.input(event);
		case sf::Event::MouseWheelMoved:
		{	
			const bool ret(camera.input(event));
			if (camera.getTotalZoom() <= 0.2f) animation.setFadeDirection(1);
			else animation.setFadeDirection(0);
			return ret;
		}
		case sf::Event::MouseButtonReleased:
			if (target)
			{
				regions[*target].highlighted = false;
				addRegionNeedingColorUpdate(*target);
			}
			target.reset(new auto(clickCheck(sf::Vector2s(event.mouseButton.x, event.mouseButton.y))));
			if (*target == unsigned short(-2)) target.reset();
			else
			{
				regions[*target].highlighted = true;
				addRegionNeedingColorUpdate(*target);
			}
			return true;
		default:
			return false;
		}
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

	const sf::Vector2s& Map::size()
	{
		return mapSize;
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

		bEnd::Region::regions[-1].sea = true;

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
				target.append(sf::Vertex(sf::Vector2f(vertex), bEnd::Region::regions.at(provID).sea ? sf::Color(56, 60, 65, 230) : sf::Color(), sf::Vector2f(vertex)));
			}
			regions.at(provID).indexEnd = target.getVertexCount();
		}
		cache.close();
		
		provinceStripes = landProvinces;
	}

	Map::Region& Map::get(const unsigned short regionID)
	{
		return regions[regionID];
	}

	void Map::initialize()
	{
		fEnd::Map::loadRegions();
		fEnd::Map::loadResources();
		fEnd::Map::launchRegionUpdateThread();
	}

	const unsigned short Map::clickCheck(sf::Vector2s point)
	{
		point = sf::Vector2s(camera.mapPixelToCoords(sf::Vector2f(point)));
		for (const auto& it : regions)
			if (bEnd::Region::get(it.first).sea)
			{
				if (utl::pointIsInsidePolygon(seaProvinces, it.second.indexBegin, it.second.indexEnd, sf::Vector2f(point)))
					return it.first;
			}
			else for (auto i(it.second.indexBegin); i != it.second.indexEnd; i += 3)
			{
				if (utl::pointIsInsideTriangle(sf::Vector2s(landProvinces[i].position), sf::Vector2s(landProvinces[i + 1].position),
					sf::Vector2s(landProvinces[i + 2].position), point)) return it.first;
			}
		return unsigned short(-2);
	}

	void Map::updateVertexArrays()
	{
		gui::TimePoint timeOfLastUpdate(gui::Internals::timeSinceStart());

		while (updateThreadLaunched)
			if (gui::Duration(gui::Internals::timeSinceStart() - timeOfLastUpdate) < gui::Duration(0.015f)) continue;			
			else if (camera.hasChanged || vertexArraysVisibilityNeedsUpdate)
			{
				timeOfLastUpdate = gui::Internals::timeSinceStart();

				const bool targetSet(!drawableBufferSet);

				const sf::FloatRect bounds(camera.getGlobalBounds().left - 30, camera.getGlobalBounds().top - 30, camera.getGlobalBounds().width + 60, camera.getGlobalBounds().height + 60);

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
					const auto& Controller(Nation::get(bEnd::Region::get(regionsNeedingColorUpdate.front()).controller)),
						Owner(Nation::get(bEnd::Region::get(regionsNeedingColorUpdate.front()).owner));
					const auto& region(regions[regionsNeedingColorUpdate.front()]);
					if (&Controller != &Owner)
						for (auto it(regions[regionsNeedingColorUpdate.front()].indexBegin), end(regions[regionsNeedingColorUpdate.front()].indexEnd); it != end; ++it)
						{
							landProvinces[it].color.r = Controller.getColor().r + (region.highlighted ? (255 - Controller.getColor().r) * 0.3f : 0);
							landProvinces[it].color.g = Controller.getColor().g + (region.highlighted ? (255 - Controller.getColor().g) * 0.3f : 0);
							landProvinces[it].color.b = Controller.getColor().b + (region.highlighted ? (255 - Controller.getColor().b) * 0.3f : 0);
							provinceStripes[it].color.r = Owner.getColor().r + (region.highlighted ? (255 - Owner.getColor().r) * 0.3f : 0);
							provinceStripes[it].color.g = Owner.getColor().g + (region.highlighted ? (255 - Owner.getColor().g) * 0.3f : 0);
							provinceStripes[it].color.b = Owner.getColor().b + (region.highlighted ? (255 - Owner.getColor().b) * 0.3f : 0);
							provinceStripes[it].color.a = 255;
						}
					else for(auto it(regions[regionsNeedingColorUpdate.front()].indexBegin), end(regions[regionsNeedingColorUpdate.front()].indexEnd); it != end; ++it)
						{
							landProvinces[it].color.r = Controller.getColor().r + (region.highlighted ? (255 - Controller.getColor().r) * 0.3f : 0);
							landProvinces[it].color.g = Controller.getColor().g + (region.highlighted ? (255 - Controller.getColor().g) * 0.3f : 0);
							landProvinces[it].color.b = Controller.getColor().b + (region.highlighted ? (255 - Controller.getColor().b) * 0.3f : 0);
							provinceStripes[it].color.a = 0;
						}
					regionsNeedingColorUpdate.pop();
				}
				colorUpdateQueueLock.unlock();
				vertexArraysVisibilityNeedsUpdate = true;
				continue;
			}
	}

	const bool isRegionAnIsland(const std::vector<std::vector<sf::Color>>& pixels, const std::vector<sf::Vector2s> contour, const sf::Color& colorCode)
	{
		if (pixels.empty() || contour.empty()) return false;

		std::vector<sf::Vector2s> pointVector;

		for (const auto& it : contour)
			if (pixels.at(it.x).at(it.y) == colorCode)
			{
				pointVector.push_back(it);
				break;
			}
		
		std::unique_ptr<sf::Color> firstBorderColor(nullptr);
		std::unordered_map<unsigned short, std::unordered_map<unsigned short, bool>> flag;

		while (!pointVector.empty())
		{
			const sf::Vector2s point(pointVector.back());
			pointVector.pop_back();

			if (!flag[point.x + 1][point.y])
			{
				if (pixels.at(point.x + 1).at(point.y) == colorCode)
					pointVector.emplace_back(point.x + 1, point.y);
				else if (firstBorderColor && *firstBorderColor != pixels.at(point.x + 1).at(point.y)) return false;
				else firstBorderColor.reset(new sf::Color(pixels.at(point.x + 1).at(point.y)));
			}

			if (!flag[point.x - 1][point.y])
			{
				if (pixels.at(point.x - 1).at(point.y) == colorCode)
					pointVector.emplace_back(point.x - 1, point.y);
				else if (firstBorderColor && *firstBorderColor != pixels.at(point.x - 1).at(point.y)) return false;
				else firstBorderColor.reset(new sf::Color(pixels.at(point.x - 1).at(point.y)));
			}

			if (!flag[point.x][point.y + 1])
			{
				if (pixels.at(point.x).at(point.y + 1) == colorCode)
					pointVector.emplace_back(point.x, point.y + 1);
				else if (firstBorderColor && *firstBorderColor != pixels.at(point.x).at(point.y + 1)) return false;
				else firstBorderColor.reset(new sf::Color(pixels.at(point.x).at(point.y + 1)));
			}

			if (!flag[point.x][point.y - 1])
			{
				if (pixels.at(point.x).at(point.y - 1) == colorCode)
					pointVector.emplace_back(point.x, point.y - 1);
				else if (firstBorderColor && *firstBorderColor != pixels.at(point.x).at(point.y - 1)) return false;
				else firstBorderColor.reset(new sf::Color(pixels.at(point.x).at(point.y - 1)));
			}

			flag[point.x][point.y] = true;
		}
		
		return true;
	}

	void cullBorderTriangles(std::vector<sf::Vector2s>& provinceContour)
	{
		if (provinceContour.size() <= 4) return;
		
		for (auto it(provinceContour.begin()); it != provinceContour.end(); ++it)
		{
			const auto A((it == provinceContour.begin() ? provinceContour.end() : it) - 1),
				B(it),
				C(it == provinceContour.end() - 1 ? provinceContour.begin() : it + 1),
				prev((A == provinceContour.begin() ? provinceContour.end() : A) - 1),
				next(C == provinceContour.end() - 1 ? provinceContour.begin() : C + 1);
			if (utl::angleType(*A, *B, *C) >= 0 && !(utl::pointsAreOnOneLine(*prev, *A, *B) &&
				utl::pointsAreOnOneLine(*B, *C, *next)) && (B->x - C->x != A->x - next->x || B->y - C->y != A->y - next->y) &&
				(B->x - A->x != C->x - prev->x || B->y - A->y != C->y - prev->y))
					it = provinceContour.erase(it);

			if (it == provinceContour.end()) break;
		}
	}

	void Map::Region::traceShape(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
		std::vector<sf::Vector2s>& borderTrianglesTarget, std::vector<std::vector<sf::Vector2s>>& contourPointsTarget)
	{
		auto points(std::move(utl::marchingSquares(pixels, colorCode)));

		for (auto& it : points)
		{
			if (it.empty()) continue;
			
			isRegionAnIsland(pixels, it, colorCode) ? cullBorderTriangles(it) :
				utl::cullBorderTriangles(pixels, colorCode, it, borderTrianglesTarget);

			contourPointsTarget.emplace_back(it);
		}
	}
}