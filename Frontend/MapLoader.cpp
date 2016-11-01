#include "MapLoader.h"

#include "../Backend/FileProcessor.h"
#include "../Backend/Province.h"

#include "../Frontend.hpp"

#include "Map.h"

namespace fEnd
{
	const sf::Color borderColor = sf::Color(21, 28, 25, 248);


	void MapLoader::loadProvinces()
	{
		unsigned short maxProvinces(0);

		{
			bEnd::FileProcessor basicProvinceDefinitions;
			if (!basicProvinceDefinitions.open("map/default.map")) return;

			//std::lock_guard<std::mutex> provincesGuard(provincesLock);
			Map::provinces[-1].sea = (bEnd::Province::provinces[-1].sea = true);
			for (auto it(basicProvinceDefinitions.getStatements().begin()), end(basicProvinceDefinitions.getStatements().end()); it != end; ++it)
				if (it->lValue == "map_size")
				{
					Map::mapSize.x = std::stoi(it->rStrings.front());
					Map::mapSize.y = std::stoi(it->rStrings.back());
				}
				else if (it->lValue == "max_provinces")
					maxProvinces = std::stoi(it->rStrings.front());
				else if (it->lValue == "sea_starts")
					for (const auto& it1 : it->rStrings)
						Map::provinces[std::stoi(it1)].sea = (bEnd::Province::provinces[std::stoi(it1)].sea = true);
		}

		if (!std::fstream("map/cache/provinces.bin").good())
		{
			console.setActive(true);
			console.print("Generating province cache - this may take a few hours.");

			std::ifstream definitions("map/definition.csv");
			if (!definitions.is_open())
			{ 
				console.print("Failed to open province definitions.");
				/*provincesLock.unlock();*/ 
				return; 
			}

			std::vector<std::vector<sf::Color>> pixels;
			{
				sf::Image bitmap;
				if (!bitmap.loadFromFile("map/provinces.bmp")) 
				{
					console.print("Failed to open province shape file.");
					/*provincesLock.unlock();*/
					return; 
				}
				pixels = std::move(utl::imageToPixelArray(bitmap));
			}

			std::vector<sf::Vector2s> unassignedBorderTriangles;
			std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>> provinceContourPoints;

			Map::provinces.reserve(maxProvinces);
			console.print("...");

			while (!definitions.eof() && Map::provinces.size() < maxProvinces - 1)
			{
				std::string buffer;

				std::getline(definitions, buffer, '\n');
				std::stringstream line(buffer);
				sf::Color provinceColor;

				std::getline(line, buffer, ';');
				const unsigned short provID(std::stoi(buffer));

				console.eraseLastLine();
				console.print(std::to_string(provID) + " of " + std::to_string(maxProvinces));

				std::getline(line, buffer, ';');
				provinceColor.r = std::stoi(buffer);
				std::getline(line, buffer, ';');
				provinceColor.g = std::stoi(buffer);
				std::getline(line, buffer, ';');
				provinceColor.b = std::stoi(buffer);

				if (provinceColor.r == 0 && provinceColor.g == 0 && provinceColor.b == 0) break;

				Map::provinces[provID].traceShape(pixels, provinceColor, unassignedBorderTriangles, provinceContourPoints[provID]);
			}

			Map::provinces[-1].traceShape(pixels, sf::Color::White, unassignedBorderTriangles, provinceContourPoints[-1]);
			//provincesLock.unlock();

			assignBorderTriangles(unassignedBorderTriangles, provinceContourPoints);

			createProvinceCache();
		}
		else loadProvinceCache();
	}
	
	void MapLoader::createProvinceCache()
	{
		std::ofstream cache("map/cache/provinces.bin", std::ios::out | std::ios::binary);

		console.print("Writing province cache.");
		//provincesLock.lock();
		for (auto it(Map::provinces.begin()), end(Map::provinces.end()); it != end; ++it)
		{
			cache.write((char*)&it->first, sizeof(it->first));

			unsigned short bytes((Map::provinces.at(it->first).indexRange.first.second - Map::provinces.at(it->first).indexRange.first.first) * 2 * 2);
			cache.write((char*)&bytes, sizeof(bytes));

			for (auto i(it->second.indexRange.first.first), end(it->second.indexRange.first.second); i < end; i++)
			{
				sf::Vector2s vertex(Map::provinceFill[i].position.x - bool(Map::provinceFill[i].position.x),
					Map::provinceFill[i].position.y - bool(Map::provinceFill[i].position.y));
				cache.write((char*)&vertex.x, sizeof(vertex.x));
				cache.write((char*)&vertex.y, sizeof(vertex.y));
			}

			bytes = (Map::provinces.at(it->first).indexRange.second.second - Map::provinces.at(it->first).indexRange.second.first) * 2 * 2;
			cache.write((char*)&bytes, sizeof(bytes));

			for (auto i(it->second.indexRange.second.first), end(it->second.indexRange.second.second); i < end; i++)
			{
				sf::Vector2s vertex(Map::provinceContours[i].position.x - bool(Map::provinceContours[i].position.x),
					Map::provinceContours[i].position.y - bool(Map::provinceContours[i].position.y));
				cache.write((char*)&vertex.x, sizeof(vertex.x));
				cache.write((char*)&vertex.y, sizeof(vertex.y));
			}
		}
		//provincesLock.unlock();
	}
	
	const sf::FloatRect getBounds(const sf::VertexArray& array, size_t begin, size_t end)
	{
		sf::FloatRect returnValue(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
		for (; begin != end; ++begin)
		{
			if (array[begin].position.x < returnValue.left) returnValue.left = array[begin].position.x;
			if (array[begin].position.y < returnValue.top) returnValue.top = array[begin].position.y;
			if (array[begin].position.x > returnValue.width) returnValue.width = array[begin].position.x;
			if (array[begin].position.y > returnValue.height) returnValue.height = array[begin].position.y;
		}
		returnValue.height -= returnValue.top;
		returnValue.width -= returnValue.left;
		return returnValue;
	}
	void MapLoader::loadProvinceCache()
	{
		std::ifstream cache("map/cache/provinces.bin", std::ios::in | std::ios::binary);
		if (!cache.is_open())
		{
			console.print("Failed to open province cache.");
			return;
		}

		console.print("Loading province cache.");
		//provincesLock.lock();
		bEnd::Province::provinces[-1].sea = true;

		while (!cache.eof())
		{
			unsigned short provID(0), bytes(0);
			cache.read((char*)&provID, sizeof(provID));
			cache.read((char*)&bytes, sizeof(bytes));

			Map::provinces[provID].indexRange.first.first = Map::provinceFill.getVertexCount();
			for (; bytes > 0; bytes -= 4)
			{
				sf::Vector2s vertex(0, 0);
				cache.read((char*)&vertex.x, sizeof(vertex.x));
				cache.read((char*)&vertex.y, sizeof(vertex.y));
				Map::provinceFill.append(sf::Vertex(sf::Vector2f(vertex), sf::Color::White, sf::Vector2f(vertex)));
			}
			Map::provinces.at(provID).indexRange.first.second = Map::provinceFill.getVertexCount();

			cache.read((char*)&(bytes = 0), sizeof(bytes));

			Map::provinces.at(provID).indexRange.second.first = Map::provinceContours.getVertexCount();
			for (; bytes > 0; bytes -= 4)
			{
				sf::Vector2s vertex(0, 0);
				cache.read((char*)&vertex.x, sizeof(vertex.x));
				cache.read((char*)&vertex.y, sizeof(vertex.y));
				Map::provinceContours.append(sf::Vertex(sf::Vector2f(vertex), borderColor, sf::Vector2f(vertex)));
			}
			Map::provinces.at(provID).indexRange.second.second = Map::provinceContours.getVertexCount();
			Map::provinces.at(provID).bounds = getBounds(Map::provinceContours, Map::provinces.at(provID).indexRange.second.first, Map::provinces.at(provID).indexRange.second.second);
		}
		//provincesLock.unlock();
		cache.close();

		Map::provinceStripes = Map::provinceFill;
	}

	void MapLoader::loadResources()
	{
		console.print("Loading map resources.");

		Map::tile.loadFromFile("map/resources/mapTile.png");
		Map::tile.setRepeated(true);
		Map::tile.setSmooth(true);

		Map::terrain.loadFromFile("map/resources/terrain.png");
		Map::terrain.setRepeated(true);
		Map::terrain.setSmooth(true);

		Map::sea.loadFromFile("map/resources/sea.png");
		Map::sea.setRepeated(true);
		Map::sea.setSmooth(true);

		Map::stripesBuffer[0].setPrimitiveType(sf::PrimitiveType::Triangles);
		Map::fillBuffer[0].setPrimitiveType(sf::PrimitiveType::Triangles);
		Map::contourBuffer[0].setPrimitiveType(sf::PrimitiveType::Lines);

		Map::stripesBuffer[1].setPrimitiveType(sf::PrimitiveType::Triangles);
		Map::fillBuffer[1].setPrimitiveType(sf::PrimitiveType::Triangles);
		Map::contourBuffer[1].setPrimitiveType(sf::PrimitiveType::Lines);

		Map::oceanGradient.clear();
		Map::oceanGradient.setPrimitiveType(sf::PrimitiveType::Quads);
		Map::oceanGradient.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(125, 130, 165, 255), sf::Vector2f(0.0f, 0.0f)));
		Map::oceanGradient.append(sf::Vertex(sf::Vector2f(Map::mapSize.x, 0.0f), sf::Color(125, 130, 165, 255), sf::Vector2f(Map::mapSize.x, 0.0f)));
		Map::oceanGradient.append(sf::Vertex(sf::Vector2f(Map::mapSize.x, Map::mapSize.y), sf::Color(135, 195, 205, 255), sf::Vector2f(Map::mapSize.x, Map::mapSize.y)));
		Map::oceanGradient.append(sf::Vertex(sf::Vector2f(0.0f, Map::mapSize.y), sf::Color(135, 195, 205, 255), sf::Vector2f(0.0f, Map::mapSize.y)));

		Map::outlineFadeAnimation.setDuration(1.0f);
		Map::outlineFadeAnimation.setFadeDirection(0);

		Map::textureTransitionAnimation.reset(new MapTransitionAnimation());
		Map::textureTransitionAnimation->setDuration(1.0f);
		Map::textureTransitionAnimation->setTextures(Map::tile);
	}

	void MapLoader::loadProvinceNames()
	{
		std::ifstream file("map/province_names.csv");
		if (!file.is_open())
		{
			console.print("Failed to open province name definitions.");
			return;
		}

		console.print("Loading province names.");
		std::string line;
		line.reserve(5);
		//provincesLock.lock();
		while (!file.eof())
		{
			line.clear();
			std::getline(file, line, ';');

			try
			{
				const auto ID(std::stoi(line));
				std::getline(file, Map::provinces[ID].name, '\n');
			}
			catch (std::exception end)
			{
				break;
			}
		}
		//provincesLock.unlock();
	}

	void MapLoader::generateWorldGraph()
	{
		if (!std::fstream("map/cache/adjacencies.bin").good())
		{
			unsigned short count(0);
			console.setActive(true);
			console.print("Generating province graph.");

			//provincesLock.lock();
			console.print(std::to_string(count) + " of " + std::to_string(Map::provinces.size()));
			for (const auto& it : Map::provinces)
			{
				if (it.first == (unsigned short)(-1)) continue;

				++count;
				console.eraseLastLine();
				console.print(std::to_string(count) + "/" + std::to_string(Map::provinces.size()));

				for (const auto& it1 : Map::provinces)
					if (it.first == it1.first || it1.first == (unsigned short)(-1)) continue;
					else if (it1.second.bounds.contains(it.second.bounds.left, it.second.bounds.top) &&
						it1.second.bounds.contains(it.second.bounds.left + it.second.bounds.width, it.second.bounds.top + it.second.bounds.top + it.second.bounds.height))
						for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; ++i)
						{
							bool found(false);
							for (size_t i1(it1.second.indexRange.first.first), end1(it1.second.indexRange.first.second); i1 < end1; i1 += 3)
								if (utl::pointIsInsideTriangle(Map::provinceFill[i1].position, Map::provinceFill[i1 + 1].position, Map::provinceFill[i1 + 2].position,
									Map::provinceFill[i].position))
								{
									connectProvinces(it.first, it1.first);
									found = true;
									break;
								}
							if (found) break;
						}
					else if (it1.second.bounds.intersects(it.second.bounds))
						for (size_t i(it.second.indexRange.second.first), end(it.second.indexRange.second.second); i < end; i += 2)
						{
							bool found(false);
							for (size_t i1(it1.second.indexRange.second.first), end1(it1.second.indexRange.second.second); i1 < end1; i1 += 2)
								if (utl::haveCommonSegment(Map::provinceContours[i].position, Map::provinceContours[i + 1].position,
									Map::provinceContours[i1].position, Map::provinceContours[i1 + 1].position))
								{
									connectProvinces(it.first, it1.first);
									found = true;
									break;
								}
							if (found) break;
						}
			}

			std::ofstream cache("map/cache/adjacencies.bin", std::ios::out | std::ios::binary);
			for (const auto& it : Map::provinces)
			{
				cache.write((char*)&it.first, sizeof(unsigned short));
				const unsigned short buffer(bEnd::Province::get(it.first).getNeighbours().size());
				cache.write((char*)&buffer, sizeof(unsigned char));
				for (const auto& it1 : bEnd::Province::get(it.first).getNeighbours())
				{
					cache.write((char*)&it1.first, sizeof(it1.first));
					cache.write((char*)&it1.second, sizeof(it1.second));
				}
			}
			//provincesLock.unlock();
		}
		else
		{
			std::ifstream cache("map/cache/adjacencies.bin", std::ios::in | std::ios::binary);
			console.print("Loading province graph.");
			while (!cache.eof())
			{
				unsigned short provID(0), neighbour(0), distance(0);
				cache.read((char*)&provID, sizeof(provID));
				unsigned char size(0);
				cache.read((char*)&size, sizeof(size));
				for (; size != 0; --size)
				{
					cache.read((char*)&neighbour, sizeof(neighbour));
					cache.read((char*)&distance, sizeof(distance));
					bEnd::Province::get(provID).neighbours.emplace(std::make_pair(neighbour, distance));
					if (!bEnd::Province::get(provID).sea && bEnd::Province::get(neighbour).sea)
						bEnd::Province::get(provID).coastal = true;
				}
			}
		}
	}


	void MapLoader::connectProvinces(const unsigned short a, const unsigned short b)
	{
		auto& it(Map::provinces.at(a)), &it1(Map::provinces.at(b));
		const auto distance(utl::distanceBetweenPoints(sf::Vector2f(it.bounds.left + it.bounds.width / 2, it.bounds.top + it.bounds.height / 2),
			sf::Vector2f(it1.bounds.left + it1.bounds.width / 2, it1.bounds.top + it1.bounds.height / 2)) * (40075 / Map::mapSize.x));
		bEnd::Province::get(a).neighbours.emplace(std::make_pair(b, distance));
		bEnd::Province::get(b).neighbours.emplace(std::make_pair(a, distance));

		if (bEnd::Province::get(a).sea && !bEnd::Province::get(b).sea)
			bEnd::Province::get(b).coastal = true;
		else if (!bEnd::Province::get(a).sea && bEnd::Province::get(b).sea)
			bEnd::Province::get(a).coastal = true;
	}

	void MapLoader::createStripesTexture(sf::Texture& targetTexture, const float size, const float stripeWidth)
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
			translate.translate(size / 4, 0);
		}

		buffer.display();

		targetTexture = buffer.getTexture();
	}

	std::pair<std::pair<std::vector<sf::Vector2s>::const_iterator, bool>, sf::Vector2s> findCommonPoints(const std::vector<sf::Vector2s>& polygon,
		const std::vector<sf::Vector2s>& triangle)
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
	void MapLoader::assignBorderTriangles(std::vector<sf::Vector2s>& unassignedTriangles, std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>>& provContour)
	{
		console.print("Finalising geometry.");
		//provincesLock.lock();
		for (auto it(provContour.begin()), end(provContour.end()); it != end; ++it)
		{
			console.eraseLastLine();
			console.print(std::to_string(it->first) + " of " + std::to_string(provContour.size()));

			Map::provinces[it->first].indexRange.first.first = Map::provinceFill.getVertexCount();
			Map::provinces.at(it->first).indexRange.second.first = Map::provinceContours.getVertexCount();

			for (auto it1(it->second.begin()), end1(it->second.end()); it1 != end1; ++it1)
			{
				bool firstPass(true);
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

				const std::vector<sf::Vector2s> simpContour(std::move(utl::simplifyShape(*it1)));
				{
					sf::Vector2f vertex(simpContour.front().x, simpContour.front().y);
					Map::provinceContours.append(sf::Vertex(vertex, borderColor, vertex));
					for (auto it2(simpContour.begin() + 1), end2(simpContour.end() - 1); it2 != end2; ++it2)
					{
						vertex.x = (it2->x);
						vertex.y = (it2->y);
						Map::provinceContours.append(sf::Vertex(vertex, borderColor, vertex));
						Map::provinceContours.append(Map::provinceContours[Map::provinceContours.getVertexCount() - 1]);
					}
					vertex.x = simpContour.back().x;
					vertex.y = simpContour.back().y;
					Map::provinceContours.append(sf::Vertex(vertex, borderColor, vertex));
				}

				{
					std::vector<sf::Vector2s> finalProvince;

					utl::tesselateShape(simpContour, finalProvince);

					for (auto it2(finalProvince.begin()), end2(finalProvince.end()); it2 != end2; ++it2)
					{
						const sf::Vector2f vertex(it2->x, it2->y);
						Map::provinceFill.append(sf::Vertex(vertex, sf::Color::White, vertex));
					}
				}
			}

			Map::provinces.at(it->first).indexRange.first.second = Map::provinceFill.getVertexCount();
			Map::provinces.at(it->first).indexRange.second.second = Map::provinceContours.getVertexCount();
		}
		//provincesLock.unlock();
		Map::provinceStripes = Map::provinceFill;
	}
}
