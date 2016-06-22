#include "Map.h"

#include "../Backend/FileProcessor.h"
#include "../Backend/Province.h"

#include "../Backend/Nation.h"
#include "Nation.h"
#include "../Frontend.hpp"
#include "utilities.h"

#include <fstream>
#include <sstream>
#include <set>

namespace fEnd
{
	const std::string stripeShaderCode =
		"uniform sampler2D mask, first, second;\
		uniform float zoomFactor, amount;\
		\
		void main()\
		{\
			gl_FragColor.rgb = (gl_Color * mix( texture2D(first, gl_TexCoord[0].xy / (2 * zoomFactor)), texture2D(second, gl_TexCoord[0].xy ), amount )).rgb;\
			gl_FragColor.a = texture2D(mask, gl_TexCoord[0].xy).a;\
		}";

	const std::string ProvinceFillTextureTransition =
		"uniform sampler2D first, second;\
		uniform float zoomFactor, amount;\
		\
		void main()\
		{\
			gl_FragColor = gl_Color * mix( texture2D(first, gl_TexCoord[0].xy / (2 * zoomFactor)), texture2D(second, gl_TexCoord[0].xy ), amount );\
		}";

	const sf::Color borderColor = sf::Color(21, 28, 25, 248);
	
	gui::FadeAnimation Map::animation;

	std::unordered_map<unsigned short, Map::Province> Map::provinces;

	sf::Vector2s Map::mapSize;

	sf::VertexArray Map::oceanGradient, Map::provinceStripes, Map::provinceFill, Map::provinceContours;
	sf::VertexArray Map::stripesBuffer[2], Map::fillBuffer[2], Map::contourBuffer[2];
	volatile std::atomic<bool> Map::drawableBufferSet, Map::vertexArraysVisibilityNeedsUpdate = false;
	std::queue<unsigned short> Map::ProvincesNeedingColorUpdate;
	std::mutex Map::colorUpdateQueueLock;

	sf::Texture Map::mapTile, Map::terrain, Map::sea, Map::stripes;
	Camera Map::camera;
	std::unique_ptr<unsigned short> Map::m_target;
	const std::unique_ptr<unsigned short>& Map::target = Map::m_target;
	std::unique_ptr<std::thread> Map::updateThread = nullptr;
	std::atomic<bool> Map::m_terminate = false;

	struct TexTransitionAnimation : public gui::Animation
	{
		TexTransitionAnimation()
		{
			fill.loadFromMemory(ProvinceFillTextureTransition, sf::Shader::Fragment);
			stripes.loadFromMemory(stripeShaderCode, sf::Shader::Fragment);
		}
		void step()const override
		{
			amount += (direction ? 1 : -1) / (getAnimationFPS() * getAnimationDuration());
			if (amount > 1.0f) amount = 1.0f;
			else if (amount < 0.0f) amount = 0.0f;
			fill.setParameter("amount", amount);
			stripes.setParameter("amount", amount);
		}
		void setTextures(const sf::Texture& first)
		{
			fill.setParameter("first", first);
			fill.setParameter("second", sf::Shader::CurrentTexture);
			stripes.setParameter("first", first);
			stripes.setParameter("second", sf::Shader::CurrentTexture);
		}
		void setMask(const sf::Texture& mask)
		{
			stripes.setParameter("mask", mask);
		}
		void setFactor(const float factor)
		{
			fill.setParameter("zoomFactor", factor);
			stripes.setParameter("zoomFactor", factor);
		}
		const sf::Shader& getFillShader()const
		{
			return fill;
		}
		const sf::Shader& getStripeShader()const
		{
			return stripes;
		}
		bool direction = 0;
	private:
		mutable sf::Shader fill, stripes;
		mutable float amount = 0;
	} fillTransitionAnimation;

	void Map::addProvinceNeedingColorUpdate(const unsigned short ProvinceID)
	{
		colorUpdateQueueLock.lock();
		ProvincesNeedingColorUpdate.emplace(ProvinceID);
		colorUpdateQueueLock.unlock();
	}

	void Map::updateAllProvinceColors()
	{
		for (auto& it : provinces)
			if (it.second.sea || bEnd::Province::get(it.first).sea) continue;
			else for (auto i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; ++i)
			{
				provinceFill[i].color = Nation::get(bEnd::Province::get(it.first).controller).getColor(); 
				if (bEnd::Province::get(it.first).controller != bEnd::Province::get(it.first).owner)
					provinceStripes[i].color = Nation::get(bEnd::Province::get(it.first).owner).getColor();
				else provinceStripes[i].color.a = 0;
			}

		vertexArraysVisibilityNeedsUpdate = true;
	}

	void Map::loadProvinces()
	{
		unsigned short maxProvinces(0);

		{
			bEnd::FileProcessor basicProvinceDefinitions;

			if (!basicProvinceDefinitions.open("map/default.map")) return;

			provinces[-1].sea = (bEnd::Province::provinces[-1].sea = true);
			for (auto it(basicProvinceDefinitions.getStatements().begin()), end(basicProvinceDefinitions.getStatements().end()); it != end; ++it)
				if (it->lValue == "map_size")
				{
					mapSize.x = std::stoi(it->rStrings.front());
					mapSize.y = std::stoi(it->rStrings.back());
				}
				else if (it->lValue == "max_Provinces")
					maxProvinces = std::stoi(it->rStrings.front());
				else if (it->lValue == "sea_starts")
					for (const auto& it1 : it->rStrings)
						provinces[std::stoi(it1)].sea = (bEnd::Province::provinces[std::stoi(it1)].sea = true);
		}

		if (!std::fstream("map/cache/provinces.bin").good())
		{
			console.print("Generating Province Cache. This may take up to 2 hours.");

			std::ifstream definitions("map/definition.csv");
			if (!definitions.is_open()) return;

			std::vector<std::vector<sf::Color>> pixels;
			{
				sf::Image bitmap;
				if (!bitmap.loadFromFile("map/provinces.bmp")) return;
				pixels = std::move(utl::imageToPixelArray(bitmap));
			}

			std::vector<sf::Vector2s> unassignedBorderTriangles;
			std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>> provinceContourPoints;

			provinces.reserve(maxProvinces);

			while (!definitions.eof() && provinces.size() < maxProvinces - 1)
			{
				std::string buffer;

				std::getline(definitions, buffer, '\n');
				std::stringstream line(buffer);
				sf::Color ProvinceColor;

				std::getline(line, buffer, ';');	
				const unsigned short provID(std::stoi(buffer));
				
				console.eraseLastLine();
				console.print(std::to_string(provID));

				std::getline(line, buffer, ';');
				ProvinceColor.r = std::stoi(buffer);
				std::getline(line, buffer, ';');
				ProvinceColor.g = std::stoi(buffer);
				std::getline(line, buffer, ';');
				ProvinceColor.b = std::stoi(buffer);

				if (ProvinceColor.r == 0 && ProvinceColor.g == 0 && ProvinceColor.b == 0) break;

				provinces[provID].traceShape(pixels, ProvinceColor, unassignedBorderTriangles, provinceContourPoints[provID]);
			}

			provinces[-1].traceShape(pixels, sf::Color::White, unassignedBorderTriangles, provinceContourPoints[-1]);
			
			assignBorderTriangles(unassignedBorderTriangles, provinceContourPoints);

			createProvinceCache();
		}
		else loadProvinceCache();
	}

	void Map::updateProvinceVisuals(const sf::Vector2s& newResolution)
	{
		const float maxZoomFactor(newResolution.y / (mapSize.y * Camera::upperZoomLimitAsMapSizeFraction));

		createStripesTexture(stripes, 64 * maxZoomFactor, 1 * maxZoomFactor);
		stripes.setRepeated(true);

		fillTransitionAnimation.setFactor(maxZoomFactor);
		fillTransitionAnimation.setMask(stripes);

		for (size_t it(0), end(provinceStripes.getVertexCount()); it != end; ++it)
		{
			provinceStripes[it].color.a = 0;
			provinceStripes[it].texCoords.x = provinceStripes[it].position.x * maxZoomFactor;
			provinceStripes[it].texCoords.y = provinceStripes[it].position.y * maxZoomFactor;
			provinceFill[it].texCoords.x = provinceFill[it].position.x * maxZoomFactor;
			provinceFill[it].texCoords.y = provinceFill[it].position.y * maxZoomFactor;
		}

		for (size_t it(0), end(oceanGradient.getVertexCount()); it != end; ++it)
		{
			oceanGradient[it].texCoords.x = oceanGradient[it].position.x * maxZoomFactor;
			oceanGradient[it].texCoords.y = oceanGradient[it].position.y * maxZoomFactor;
		}
		
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
			translate.translate(size / 4, 0);
		}

		buffer.display();

		targetTexture = buffer.getTexture();
	}

	void Map::loadResources()
	{
		mapTile.loadFromFile("map/resources/mapTile.png");
		mapTile.setRepeated(true);
		mapTile.setSmooth(true);

		terrain.loadFromFile("map/resources/terrain.png");
		terrain.setRepeated(true);
		terrain.setSmooth(true);

		sea.loadFromFile("map/resources/sea.png");
		sea.setRepeated(true);
		sea.setSmooth(true);
		
		stripesBuffer[0].setPrimitiveType(sf::PrimitiveType::Triangles);
		fillBuffer[0].setPrimitiveType(sf::PrimitiveType::Triangles);
		contourBuffer[0].setPrimitiveType(sf::PrimitiveType::Lines);

		stripesBuffer[1].setPrimitiveType(sf::PrimitiveType::Triangles);
		fillBuffer[1].setPrimitiveType(sf::PrimitiveType::Triangles);
		contourBuffer[1].setPrimitiveType(sf::PrimitiveType::Lines);

		oceanGradient.clear();
		oceanGradient.setPrimitiveType(sf::PrimitiveType::Quads);
		oceanGradient.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(125, 130, 165, 255), sf::Vector2f(0.0f, 0.0f)));
		oceanGradient.append(sf::Vertex(sf::Vector2f(mapSize.x, 0.0f), sf::Color(125, 130, 165, 255), sf::Vector2f(mapSize.x, 0.0f)));
		oceanGradient.append(sf::Vertex(sf::Vector2f(mapSize.x, mapSize.y), sf::Color(135, 195, 205, 255), sf::Vector2f(mapSize.x, mapSize.y)));
		oceanGradient.append(sf::Vertex(sf::Vector2f(0.0f, mapSize.y), sf::Color(135, 195, 205, 255), sf::Vector2f(0.0f, mapSize.y)));

		animation.setDuration(1.0f);
		animation.setFadeDirection(0);
		fillTransitionAnimation.setDuration(1.0f);
		fillTransitionAnimation.setTextures(mapTile);
	}

	const sf::FloatRect Map::getViewBounds()
	{
		return camera.getGlobalBounds();
	}

	void Map::setViewPosition(const float x, const float y)
	{
		camera.setPosition(x, y);
	}

	volatile std::atomic<bool> updatingColors = false;

	void Map::waitForColorUpdate()
	{
		if (ProvincesNeedingColorUpdate.empty()) return;
		updatingColors = true;
		while (updatingColors);
	}

	void Map::select(const unsigned short id)
	{
		deselect();
		if (!provinces.count(id) || id == -2 || provinces.at(id).sea) return;
		m_target.reset(new auto(id));

		if (fEnd::currentScreen == Game)
		{
			provinces[*m_target].highlighted = true;
			addProvinceNeedingColorUpdate(*m_target);
		}
		else
		{
			const auto controller(bEnd::Province::get(*m_target).getController());
			for (auto& it : provinces)
				if (bEnd::Province::get(it.first).getController() == controller)
				{
					it.second.highlighted = true;
					addProvinceNeedingColorUpdate(it.first);
				}
			waitForColorUpdate();
			bEnd::Nation::player = bEnd::Province::get(*m_target).getController();
		}
	}

	void Map::deselect()
	{
		if (!m_target) return;
		if (fEnd::currentScreen == Game)
		{
			provinces[*m_target].highlighted = false;
			addProvinceNeedingColorUpdate(*m_target);
		}
		else
		{
			const auto controller(bEnd::Province::get(*m_target).getController());
			for (auto& it : provinces)
				if (bEnd::Province::get(it.first).getController() == controller)
				{
					it.second.highlighted = false;
					addProvinceNeedingColorUpdate(it.first);
				}
		}
		m_target.reset();
	}

	void Map::terminate()
	{
		if (updateThread)
		{
			m_terminate = true;
			updateThread->join();
			updateThread.reset();
		}
	}

	void Map::draw(sf::RenderTarget& target, sf::RenderStates states)const
	{
		target.draw(camera);
		target.draw(fillTransitionAnimation);

		states.texture = &sea;
		states.shader = &fillTransitionAnimation.getFillShader();
		if (camera.getPosition().x < 0)
		{
			states.transform.translate(-mapSize.x, 0);
			target.draw(oceanGradient, states);
			states.transform.translate(mapSize.x, 0);
		}

		target.draw(oceanGradient, states);
		states.texture = &terrain;
		target.draw(fillBuffer[drawableBufferSet], states);
		states.shader = &fillTransitionAnimation.getStripeShader();
		target.draw(stripesBuffer[drawableBufferSet], states);

		target.draw(animation);
		if (animation.getFadeAmount() != 0.0f)
			target.draw(contourBuffer[drawableBufferSet], &animation.getShaderNonTextured());
		target.setView(target.getDefaultView());
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
			if (camera.getTotalZoom() <= 0.2f) animation.setFadeDirection(fillTransitionAnimation.direction = 1);
			else animation.setFadeDirection(fillTransitionAnimation.direction = 0);
			return ret;
		}
		case sf::Event::MouseButtonReleased:
			select(clickCheck(sf::Vector2s(event.mouseButton.x, event.mouseButton.y)));
			return true;
		case sf::Event::KeyReleased:
			if (event.key.code == sf::Keyboard::F4 && event.key.alt)
			{
				terminate();
				std::exit(0);
			}
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

	void Map::assignBorderTriangles(std::vector<sf::Vector2s>& unassignedTriangles, std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>>& provContour)
	{
		for (auto it(provContour.begin()), end(provContour.end()); it != end; ++it)
		{
			console.eraseLastLine();
			console.print(std::to_string(it->first));

			provinces[it->first].indexRange.first.first = provinceFill.getVertexCount();
			provinces.at(it->first).indexRange.second.first = provinceContours.getVertexCount();

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
					provinceContours.append(sf::Vertex(vertex, borderColor, vertex));
					for (auto it2(simpContour.begin() + 1), end2(simpContour.end() - 1); it2 != end2; ++it2)
					{
						vertex.x = (it2->x);
						vertex.y = (it2->y);
						provinceContours.append(sf::Vertex(vertex, borderColor, vertex));
						provinceContours.append(provinceContours[provinceContours.getVertexCount() - 1]);
					}
					vertex.x = simpContour.back().x;
					vertex.y = simpContour.back().y;
					provinceContours.append(sf::Vertex(vertex, borderColor, vertex));
				}

				{
					std::vector<sf::Vector2s> finalProvince;

					utl::tesselateShape(simpContour, finalProvince);

					for (auto it2(finalProvince.begin()), end2(finalProvince.end()); it2 != end2; ++it2)
					{
						const sf::Vector2f vertex(it2->x, it2->y);
						provinceFill.append(sf::Vertex(vertex, sf::Color::White, vertex));
					}
				}
			}

			provinces.at(it->first).indexRange.first.second = provinceFill.getVertexCount();
			provinces.at(it->first).indexRange.second.second = provinceContours.getVertexCount();
		}
		provinceStripes = provinceFill;
	}
	
	void Map::createProvinceCache()
	{
		std::ofstream cache("map/cache/provinces.bin", std::ios::out | std::ios::binary);

		for (auto it(provinces.begin()), end(provinces.end()); it != end; ++it)
		{
			cache.write((char*)&it->first, sizeof(it->first));

			unsigned short bytes((provinces.at(it->first).indexRange.first.second - provinces.at(it->first).indexRange.first.first) * 2 * 2);
			cache.write((char*)&bytes, sizeof(bytes));

			for (auto i(it->second.indexRange.first.first), end(it->second.indexRange.first.second); i < end; i++)
			{
				sf::Vector2s vertex(provinceFill[i].position.x - bool(provinceFill[i].position.x), provinceFill[i].position.y - bool(provinceFill[i].position.y));
				cache.write((char*)&vertex.x, sizeof(vertex.x));
				cache.write((char*)&vertex.y, sizeof(vertex.y));
			}

			bytes = (provinces.at(it->first).indexRange.second.second - provinces.at(it->first).indexRange.second.first) * 2 * 2;
			cache.write((char*)&bytes, sizeof(bytes));

			for (auto i(it->second.indexRange.second.first), end(it->second.indexRange.second.second); i < end; i++)
			{
				sf::Vector2s vertex(provinceContours[i].position.x - bool(provinceContours[i].position.x), provinceContours[i].position.y - bool(provinceContours[i].position.y));
				cache.write((char*)&vertex.x, sizeof(vertex.x));
				cache.write((char*)&vertex.y, sizeof(vertex.y));
			}
		}
	}

	const sf::FloatRect getBounds(const sf::VertexArray& array, size_t begin, size_t end)
	{
		sf::FloatRect returnValue;
		for (;begin != end; ++begin)
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

	void Map::loadProvinceCache()
	{
		std::ifstream cache("map/cache/provinces.bin", std::ios::in | std::ios::binary);
		bEnd::Province::provinces[-1].sea = true;

		while (!cache.eof())
		{
			unsigned short provID(0), bytes(0);
			cache.read((char*)&provID, sizeof(provID));
			cache.read((char*)&bytes, sizeof(bytes));

			provinces[provID].indexRange.first.first = provinceFill.getVertexCount();
			for (bytes; bytes > 0; bytes -= 4)
			{
				sf::Vector2s vertex(0, 0);
				cache.read((char*)&vertex.x, sizeof(vertex.x));
				cache.read((char*)&vertex.y, sizeof(vertex.y));
				provinceFill.append(sf::Vertex(sf::Vector2f(vertex), sf::Color::White, sf::Vector2f(vertex)));
			}
			provinces.at(provID).indexRange.first.second = provinceFill.getVertexCount();

			cache.read((char*)&(bytes = 0), sizeof(bytes));

			provinces.at(provID).indexRange.second.first = provinceContours.getVertexCount();
			for (bytes; bytes > 0; bytes -= 4)
			{
				sf::Vector2s vertex(0, 0);
				cache.read((char*)&vertex.x, sizeof(vertex.x));
				cache.read((char*)&vertex.y, sizeof(vertex.y));
				provinceContours.append(sf::Vertex(sf::Vector2f(vertex), borderColor, sf::Vector2f(vertex)));
			}
			provinces.at(provID).indexRange.second.second = provinceContours.getVertexCount();
			provinces.at(provID).bounds = getBounds(provinceContours, provinces.at(provID).indexRange.second.first, provinces.at(provID).indexRange.second.second);
		}
		cache.close();

		provinceStripes = provinceFill;
	}

	void Map::loadProvinceNames()
	{
		std::ifstream file("map/province_names.csv");
		if (!file.is_open()) return;

		std::string line;
		line.reserve(5);
		while (!file.eof())
		{
			line.clear();
			std::getline(file, line, ';');

			try
			{
				const auto ID(std::stoi(line));
				std::getline(file, provinces[ID].name, '\n');
			}
			catch (std::exception end)
			{
				break;
			}
		}
	}
	
	void Map::generateWorldGraph()
	{
		if (!std::fstream("map/cache/adjacencies.bin").good())
		{
			unsigned short count(0);
			console.print("Generating Province Graph...");
			console.print(std::to_string(count) + "/" + std::to_string(provinces.size()));
			for (const auto& it : provinces)
			{
				++count;
				console.eraseLastLine();
				console.print(std::to_string(count) + "/" + std::to_string(provinces.size()));

				for (const auto& it1 : provinces)
					if (it.first == it1.first) continue;
					else for (size_t i(it.second.indexRange.second.first), end(it.second.indexRange.second.second); i < end; i += 2)
					{
						bool found(false);
						for (size_t i1(it1.second.indexRange.second.first), end1(it1.second.indexRange.second.second); i1 < end1; i1 += 2)
							if (utl::haveCommonSegment(provinceContours[i].position, provinceContours[i + 1].position,
								provinceContours[i1].position, provinceContours[i1 + 1].position))
							{
								const auto distance(utl::distanceBetweenPoints(sf::Vector2f(it.second.bounds.left + it.second.bounds.width / 2, it.second.bounds.top + it.second.bounds.height / 2),
									sf::Vector2f(it1.second.bounds.left + it1.second.bounds.width / 2, it1.second.bounds.top + it1.second.bounds.height / 2)));
								bEnd::Province::get(it.first).neighbours.emplace(std::make_pair(it1.first, distance * (40075.0f / mapSize.x)));
								bEnd::Province::get(it1.first).neighbours.emplace(std::make_pair(it.first, distance* (40075.0f / mapSize.y)));
								found = true;
								break;
							}
						if (found) break;
					}
			}

			std::ofstream cache("map/cache/adjacencies.bin", std::ios::out | std::ios::binary);
			for (const auto& it : provinces)
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
		}
		else
		{
			console.print("Loading Province Graph...");
			std::ifstream cache("map/cache/adjacencies.bin", std::ios::in | std::ios::binary);
			while (!cache.eof())
			{
				unsigned short provID(0), neighbour(0), distance(0);
				cache.read((char*)&provID, sizeof(provID));
				unsigned char size(0);
				cache.read((char*)&size, sizeof(size));
				for (;size != 0; --size)
				{
					cache.read((char*)&neighbour, sizeof(neighbour));
					cache.read((char*)&distance, sizeof(distance));
					bEnd::Province::get(provID).neighbours.emplace(std::make_pair(neighbour, distance));
				}
			}
		}
	}

	Map::Province& Map::get(const unsigned short ProvinceID)
	{
		return provinces[ProvinceID];
	}

	void Map::initialise()
	{
		console.print("Loading Map...");

		loadProvinces();
		generateWorldGraph();
		loadResources();
		loadProvinceNames();
		updateThread.reset(new std::thread(updateVertexArrays));

		console.print("Done.");
	}

	const unsigned short Map::clickCheck(sf::Vector2s point)
	{
		point = sf::Vector2s(camera.mapPixelToCoords(sf::Vector2f(point)));
		for (const auto& it : provinces)
			if (it.second.sea) continue;
			else for (size_t i(it.second.indexRange.first.first); i != it.second.indexRange.first.second; i += 3)
				if (utl::pointIsInsideTriangle(sf::Vector2s(provinceFill[i].position), sf::Vector2s(provinceFill[i + 1].position),
					sf::Vector2s(provinceFill[i + 2].position), point) || utl::pointIsInsideTriangle(sf::Vector2s(provinceFill[i].position), sf::Vector2s(provinceFill[i + 1].position),
						sf::Vector2s(provinceFill[i + 2].position), sf::Vector2s(point.x + mapSize.x, point.y)))
						return it.first;
		for (const auto& it : provinces)
			if (!it.second.sea) continue;
			else for (size_t i(it.second.indexRange.first.first); i != it.second.indexRange.first.second; i += 3)
				if (utl::pointIsInsideTriangle(sf::Vector2s(provinceFill[i].position), sf::Vector2s(provinceFill[i + 1].position),
					sf::Vector2s(provinceFill[i + 2].position), point) || utl::pointIsInsideTriangle(sf::Vector2s(provinceFill[i].position), sf::Vector2s(provinceFill[i + 1].position),
						sf::Vector2s(provinceFill[i + 2].position), sf::Vector2s(point.x + mapSize.x, point.y)))
					return it.first;
		return unsigned short(-2);
	}

	void Map::updateVertexArrays()
	{
		gui::TimePoint timeOfLastUpdate(gui::Internals::timeSinceStart());

		while (!m_terminate)
		{
			if (gui::Duration(gui::Internals::timeSinceStart() - timeOfLastUpdate) < gui::Duration(0.015f)) continue;
			else if (camera.hasChanged || vertexArraysVisibilityNeedsUpdate)
			{
				timeOfLastUpdate = gui::Internals::timeSinceStart();

				const bool targetSet(!drawableBufferSet);

				const sf::FloatRect bounds(camera.getGlobalBounds().left - 30, camera.getGlobalBounds().top - 30,
					camera.getGlobalBounds().width + 60, camera.getGlobalBounds().height + 60);

				fillBuffer[targetSet].clear();
				stripesBuffer[targetSet].clear();
				contourBuffer[targetSet].clear();

				if (animation.getFadeAmount() != 0.0f)
				{
					if (bounds.left < 0)
					{
						if (camera.getTotalZoom() <= 0.2f)
							for (size_t i(0), end(provinceContours.getVertexCount()); i < end; i += 2)
							{
								if (bounds.contains(provinceContours[i].position.x - mapSize.x, provinceContours[i].position.y) ||
									bounds.contains(provinceContours[i + 1].position.x - mapSize.x, provinceContours[i + 1].position.y))
								{
									contourBuffer[targetSet].append(sf::Vertex(sf::Vector2f(provinceContours[i].position.x - mapSize.x, provinceContours[i].position.y),
										provinceContours[i].color, provinceContours[i].texCoords));
									contourBuffer[targetSet].append(sf::Vertex(sf::Vector2f(provinceContours[i + 1].position.x - mapSize.x, provinceContours[i + 1].position.y),
										provinceContours[i + 1].color, provinceContours[i + 1].texCoords));
								}
								else if (bounds.contains(provinceContours[i].position) ||
									bounds.contains(provinceContours[i + 1].position))
								{
									contourBuffer[targetSet].append(provinceContours[i]);
									contourBuffer[targetSet].append(provinceContours[i + 1]);
								}
							}
					}
					else for (size_t i(0), end(provinceContours.getVertexCount()); i < end; i += 2)
						if (bounds.contains(provinceContours[i].position) ||
							bounds.contains(provinceContours[i + 1].position))
						{
							contourBuffer[targetSet].append(provinceContours[i]);
							contourBuffer[targetSet].append(provinceContours[i + 1]);
						}

				}

				for (const auto& it : provinces)
				{
					if (it.second.sea || bEnd::Province::get(it.first).sea)
					{
						if (!it.second.visible) continue;
						for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; i += 3)
							if ((bounds.contains(provinceFill[i].position) ||
								bounds.contains(provinceFill[i + 1].position) ||
								bounds.contains(provinceFill[i + 2].position)))
							{
								fillBuffer[targetSet].append(provinceFill[i]);
								fillBuffer[targetSet].append(provinceFill[i + 1]);
								fillBuffer[targetSet].append(provinceFill[i + 2]);
							}
					}
					else for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; i += 3)
						if (bounds.contains(provinceFill[i].position) ||
							bounds.contains(provinceFill[i + 1].position) ||
							bounds.contains(provinceFill[i + 2].position))
						{
							fillBuffer[targetSet].append(provinceFill[i]);
							fillBuffer[targetSet].append(provinceFill[i + 1]);
							fillBuffer[targetSet].append(provinceFill[i + 2]);

							if (provinceStripes[i].color.a != 0)
							{
								stripesBuffer[targetSet].append(provinceStripes[i]);
								stripesBuffer[targetSet].append(provinceStripes[i + 1]);
								stripesBuffer[targetSet].append(provinceStripes[i + 2]);
							}
						}

					if (bounds.left < 0)
					{
						const sf::Vertex* vertex;
						if (it.second.sea)
						{
							if (!it.second.visible) continue;
							for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; i += 3)
								if ((bounds.contains(provinceFill[i].position) ||
									bounds.contains(provinceFill[i + 1].position) ||
									bounds.contains(provinceFill[i + 2].position)))
								{
									vertex = &provinceFill[i];
									fillBuffer[targetSet].append(sf::Vertex(sf::Vector2f(vertex->position.x - mapSize.x, vertex->position.y),
										vertex->color, vertex->texCoords));
									fillBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &provinceFill[i + 1])->position.x - mapSize.x, vertex->position.y),
										vertex->color, vertex->texCoords));
									fillBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &provinceFill[i + 2])->position.x - mapSize.x, vertex->position.y),
										vertex->color, vertex->texCoords));
								}
						}
						else for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; i += 3)
							if (bounds.contains(provinceFill[i].position.x - mapSize.x, provinceFill[i].position.y) ||
								bounds.contains(provinceFill[i + 1].position.x - mapSize.x, provinceFill[i + 1].position.y) ||
								bounds.contains(provinceFill[i + 2].position.x - mapSize.x, provinceFill[i + 2].position.y))
							{
								vertex = &provinceFill[i];
								fillBuffer[targetSet].append(sf::Vertex(sf::Vector2f(vertex->position.x - mapSize.x, vertex->position.y),
									vertex->color, vertex->texCoords));
								fillBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &provinceFill[i + 1])->position.x - mapSize.x, vertex->position.y),
									vertex->color, vertex->texCoords));
								fillBuffer[targetSet].append(sf::Vertex(sf::Vector2f((vertex = &provinceFill[i + 2])->position.x - mapSize.x, vertex->position.y),
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
					}
				}

				drawableBufferSet = !drawableBufferSet;
				camera.hasChanged = false;
				vertexArraysVisibilityNeedsUpdate = false;
				timeOfLastUpdate = gui::Internals::timeSinceStart();
			}
			else processUpdateQueue();
		}
	}

	void Map::processUpdateQueue()
	{
		colorUpdateQueueLock.lock();
		while (ProvincesNeedingColorUpdate.size() != 0)
		{
			if (bEnd::Province::get(ProvincesNeedingColorUpdate.front()).sea)
			{
				ProvincesNeedingColorUpdate.pop();
				continue;
			}
			const auto& Controller(Nation::get(bEnd::Province::get(ProvincesNeedingColorUpdate.front()).controller)),
				Owner(Nation::get(bEnd::Province::get(ProvincesNeedingColorUpdate.front()).owner));
			const auto& Province(provinces[ProvincesNeedingColorUpdate.front()]);
			if (bEnd::Province::get(ProvincesNeedingColorUpdate.front()).controller != bEnd::Province::get(ProvincesNeedingColorUpdate.front()).owner)
				for (auto it(provinces[ProvincesNeedingColorUpdate.front()].indexRange.first.first),
					end(provinces[ProvincesNeedingColorUpdate.front()].indexRange.first.second); it != end; ++it)
				{
					provinceFill[it].color.r = Controller.getColor().r + (Province.highlighted ? (255 - Controller.getColor().r) * 0.3f : 0);
					provinceFill[it].color.g = Controller.getColor().g + (Province.highlighted ? (255 - Controller.getColor().g) * 0.3f : 0);
					provinceFill[it].color.b = Controller.getColor().b + (Province.highlighted ? (255 - Controller.getColor().b) * 0.3f : 0);
					provinceStripes[it].color.r = Owner.getColor().r + (Province.highlighted ? (255 - Owner.getColor().r) * 0.3f : 0);
					provinceStripes[it].color.g = Owner.getColor().g + (Province.highlighted ? (255 - Owner.getColor().g) * 0.3f : 0);
					provinceStripes[it].color.b = Owner.getColor().b + (Province.highlighted ? (255 - Owner.getColor().b) * 0.3f : 0);
					provinceStripes[it].color.a = 255;
				}
			else for (auto it(provinces[ProvincesNeedingColorUpdate.front()].indexRange.first.first),
				end(provinces[ProvincesNeedingColorUpdate.front()].indexRange.first.second); it != end; ++it)
			{
				provinceFill[it].color.r = Controller.getColor().r + (Province.highlighted ? (255 - Controller.getColor().r) * 0.3f : 0);
				provinceFill[it].color.g = Controller.getColor().g + (Province.highlighted ? (255 - Controller.getColor().g) * 0.3f : 0);
				provinceFill[it].color.b = Controller.getColor().b + (Province.highlighted ? (255 - Controller.getColor().b) * 0.3f : 0);
				provinceStripes[it].color.a = 0;
			}
			ProvincesNeedingColorUpdate.pop();
		}
		colorUpdateQueueLock.unlock();
		updatingColors = false;
		vertexArraysVisibilityNeedsUpdate = true;
	}

	const bool isProvinceAnIsland(const std::vector<std::vector<sf::Color>>& pixels, const std::vector<sf::Vector2s> contour, const sf::Color& colorCode)
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

	void cullBorderTriangles(std::vector<sf::Vector2s>& ProvinceContour)
	{
		if (ProvinceContour.size() <= 4) return;
		
		for (auto it(ProvinceContour.begin()); it != ProvinceContour.end(); ++it)
		{
			const auto A((it == ProvinceContour.begin() ? ProvinceContour.end() : it) - 1),
				B(it),
				C(it == ProvinceContour.end() - 1 ? ProvinceContour.begin() : it + 1),
				prev((A == ProvinceContour.begin() ? ProvinceContour.end() : A) - 1),
				next(C == ProvinceContour.end() - 1 ? ProvinceContour.begin() : C + 1);
			if (utl::angleType(*A, *B, *C) >= 0 && !(utl::pointsAreOnOneLine(*prev, *A, *B) &&
				utl::pointsAreOnOneLine(*B, *C, *next)) && (B->x - C->x != A->x - next->x || B->y - C->y != A->y - next->y) &&
				(B->x - A->x != C->x - prev->x || B->y - A->y != C->y - prev->y))
					it = ProvinceContour.erase(it);

			if (it == ProvinceContour.end()) break;
		}
	}

	void Map::Province::traceShape(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
		std::vector<sf::Vector2s>& borderTrianglesTarget, std::vector<std::vector<sf::Vector2s>>& contourPointsTarget)
	{
		auto points(std::move(utl::marchingSquares(pixels, colorCode)));

		for (auto& it : points)
		{
			if (it.empty()) continue;

			isProvinceAnIsland(pixels, it, colorCode) ? cullBorderTriangles(it) :
				utl::cullBorderTriangles(pixels, colorCode, it, borderTrianglesTarget);
			contourPointsTarget.emplace_back(it);
		}
	}
}