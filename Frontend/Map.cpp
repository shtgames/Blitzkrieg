#include "Map.h"

#include "../Backend/FileProcessor.h"
#include "../Backend/Province.h"
#include "../Backend/Nation.h"

#include "Nation.h"
#include "MapLoader.h"
#include "utilities.h"

#include "../Frontend.hpp"

#include <fstream>
#include <sstream>
#include <set>
#include <limits>

namespace fEnd
{
	Camera Map::camera;

	gui::FadeAnimation Map::outlineFadeAnimation;
	std::unique_ptr<MapTransitionAnimation> Map::textureTransitionAnimation;

	std::unordered_map<unsigned short, Province> Map::provinces;

	sf::Vector2s Map::mapSize;

	sf::VertexArray Map::oceanGradient, Map::provinceStripes, Map::provinceFill, Map::provinceContours;
	sf::VertexArray Map::stripesBuffer[2], Map::fillBuffer[2], Map::contourBuffer[2];
	volatile std::atomic<bool> Map::drawableBufferSet = {0}, Map::vertexArraysVisibilityNeedsUpdate = {false};

	std::queue<unsigned short> Map::provincesNeedingColorUpdate;
	std::mutex Map::colorUpdateQueueLock/*, Map::provincesLock*/;

	std::unique_ptr<unsigned short> Map::m_target;
	const std::unique_ptr<unsigned short>& Map::target = Map::m_target;

	sf::Texture Map::tile, Map::terrain, Map::sea, Map::stripes;

	std::atomic<bool> Map::m_terminate = {false};
	std::unique_ptr<boost::thread> Map::updateThread = nullptr;


	void Map::initialise()
	{
		console.print("Loading map.");

		MapLoader::loadProvinces();
		MapLoader::generateWorldGraph();
		MapLoader::loadResources();
		MapLoader::loadProvinceNames();

		updateThread.reset(new boost::thread(updateVertexArrays));
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


	std::unique_ptr<gui::Interactive> Map::copy() const
	{
		return std::unique_ptr<Map>(new Map(*this));
	}

	std::unique_ptr<gui::Interactive> Map::move()
	{
		return std::unique_ptr<Map>(new Map(std::move(*this)));
	}


	const sf::FloatRect Map::getViewBounds()
	{
		return camera.getGlobalBounds();
	}

	const sf::FloatRect Map::getGlobalBounds() const
	{
		return camera.getGlobalBounds();
	}

	const sf::Vector2f& Map::getPosition() const
	{
		return camera.getPosition();
	}

	Province& Map::get(const unsigned short ProvinceID)
	{
		//std::lock_guard<std::mutex> provincesGuard(provincesLock);
		return provinces[ProvinceID];
	}

	const sf::Vector2s& Map::size()
	{
		return mapSize;
	}


	void Map::setViewPosition(const float x, const float y)
	{
		camera.setPosition(x, y);
	}

	Map& Map::setPosition(const float x, const float y)
	{
		camera.setPosition(x, y);
		return *this;
	}


	void Map::select(const unsigned short id)
	{
		deselect();

		//std::lock_guard<std::mutex> provincesGuard(provincesLock);
		if (!provinces.count(id) || id == -2 || provinces.at(id).sea) return;
		m_target.reset(new auto(id));

		if (fEnd::currentScreen == Game)
		{
			provinces[*m_target].highlighted = true;
			addProvinceNeedingColorUpdate(*m_target);
		}
		else textureTransitionAnimation->setHighlightedColor(Nation::get(bEnd::Nation::player = bEnd::Province::get(*m_target).getController()).getColor());
	}

	void Map::deselect()
	{
		if (!m_target) return;
		if (fEnd::currentScreen == Game)
		{
			//provincesLock.lock();
			provinces[*m_target].highlighted = false;
			//provincesLock.unlock();
			addProvinceNeedingColorUpdate(*m_target);
		}
		else textureTransitionAnimation->setHighlightedColor(sf::Color(0, 0, 0, 0));
		m_target.reset();
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
			if (camera.getTotalZoom() <= 0.2f) outlineFadeAnimation.setFadeDirection(textureTransitionAnimation->direction = 1);
			else outlineFadeAnimation.setFadeDirection(textureTransitionAnimation->direction = 0);
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

	const unsigned short Map::clickCheck(sf::Vector2s point1)
	{
		sf::Vector2f point = camera.mapPixelToCoords(sf::Vector2f(point1));
		//std::lock_guard<std::mutex> provincesGuard(provincesLock);
		for (const auto& it : provinces)
			if (it.second.sea) continue;
			else if (it.second.bounds.contains(point))
				for (size_t i(it.second.indexRange.first.first); i != it.second.indexRange.first.second; i += 3)
					if (utl::pointIsInsideTriangle(provinceFill[i].position, provinceFill[i + 1].position,
						provinceFill[i + 2].position, point) || utl::pointIsInsideTriangle(provinceFill[i].position, provinceFill[i + 1].position,
							provinceFill[i + 2].position, sf::Vector2f(point.x + mapSize.x, point.y)))
						return it.first;
		for (const auto& it : provinces)
			if (!it.second.sea) continue;
			else if (it.second.bounds.contains(point))
				for (size_t i(it.second.indexRange.first.first); i != it.second.indexRange.first.second; i += 3)
					if (utl::pointIsInsideTriangle(provinceFill[i].position, provinceFill[i + 1].position,
						provinceFill[i + 2].position, point) || utl::pointIsInsideTriangle(provinceFill[i].position, provinceFill[i + 1].position,
							provinceFill[i + 2].position, sf::Vector2f(point.x + mapSize.x, point.y)))
						return it.first;
		return (unsigned short)(-2);
	}


	void Map::draw(sf::RenderTarget& target, sf::RenderStates states)const
	{
		target.draw(camera);
		target.draw(*textureTransitionAnimation);

		states.texture = &sea;
		states.shader = &textureTransitionAnimation->getFillShader();
		if (camera.getPosition().x < 0)
		{
			states.transform.translate(-mapSize.x, 0);
			target.draw(oceanGradient, states);
			states.transform.translate(mapSize.x, 0);
		}

		target.draw(oceanGradient, states);
		states.texture = &terrain;
		target.draw(fillBuffer[drawableBufferSet], states);
		states.shader = &textureTransitionAnimation->getStripeShader();
		target.draw(stripesBuffer[drawableBufferSet], states);

		target.draw(outlineFadeAnimation);
		if (outlineFadeAnimation.getFadeAmount() != 0.0f)
			target.draw(contourBuffer[drawableBufferSet], &outlineFadeAnimation.getShaderNonTextured());
		target.setView(target.getDefaultView());
	}


	volatile std::atomic<bool> updatingColors = {false};
	void Map::waitForColorUpdate()
	{
		if (provincesNeedingColorUpdate.empty()) return;
		updatingColors = true;
		while (updatingColors);
	}

	void Map::addProvinceNeedingColorUpdate(const unsigned short ProvinceID)
	{
		colorUpdateQueueLock.lock();
		provincesNeedingColorUpdate.emplace(ProvinceID);
		colorUpdateQueueLock.unlock();
	}

	void Map::updateAllProvinceColors()
	{
		//std::lock_guard<std::mutex> provincesGuard(provincesLock);
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

	void Map::updateProvinceVisuals(const sf::Vector2s& newResolution)
	{
		const float maxZoomFactor(newResolution.y / (mapSize.y * Camera::upperZoomLimitAsMapSizeFraction));

		MapLoader::createStripesTexture(stripes, 64 * maxZoomFactor, 1 * maxZoomFactor);
		stripes.setRepeated(true);

		textureTransitionAnimation->setFactor(maxZoomFactor);
		textureTransitionAnimation->setMask(stripes);

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

	inline const sf::FloatRect translateRect(sf::FloatRect rect, const float amount)
	{
		rect.left += amount;
		return rect;
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

				if (outlineFadeAnimation.getFadeAmount() != 0.0f)
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

				//provincesLock.lock();
				for (const auto& it : provinces)
				{
					if (it.second.sea || bEnd::Province::get(it.first).sea)
					{
						if (!it.second.visible) continue;
						if (bounds.intersects(it.second.bounds))
							for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; i += 3)
							{
								fillBuffer[targetSet].append(provinceFill[i]);
								fillBuffer[targetSet].append(provinceFill[i + 1]);
								fillBuffer[targetSet].append(provinceFill[i + 2]);
							}
					}
					else if (bounds.intersects(it.second.bounds))
						for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; i += 3)
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
							if (bounds.intersects(translateRect(it.second.bounds, -mapSize.x)))
								for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; i += 3)
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
						else if (bounds.intersects(translateRect(it.second.bounds, -mapSize.x)))
							for (size_t i(it.second.indexRange.first.first), end(it.second.indexRange.first.second); i < end; i += 3)
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
				//provincesLock.unlock();

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
		while (provincesNeedingColorUpdate.size() != 0)
		{
			if (bEnd::Province::get(provincesNeedingColorUpdate.front()).sea)
			{
				provincesNeedingColorUpdate.pop();
				continue;
			}
			
			const auto& controller(Nation::get(bEnd::Province::get(provincesNeedingColorUpdate.front()).controller)),
				&owner(Nation::get(bEnd::Province::get(provincesNeedingColorUpdate.front()).owner));

			//provincesLock.lock();
			const auto& Province(provinces[provincesNeedingColorUpdate.front()]);

			if (bEnd::Province::get(provincesNeedingColorUpdate.front()).controller != bEnd::Province::get(provincesNeedingColorUpdate.front()).owner)
				for (auto it(provinces[provincesNeedingColorUpdate.front()].indexRange.first.first),
					end(provinces[provincesNeedingColorUpdate.front()].indexRange.first.second); it != end; ++it)
				{
					provinceFill[it].color.r = controller.getColor().r + (Province.highlighted ? (255 - controller.getColor().r) * 0.3f : 0);
					provinceFill[it].color.g = controller.getColor().g + (Province.highlighted ? (255 - controller.getColor().g) * 0.3f : 0);
					provinceFill[it].color.b = controller.getColor().b + (Province.highlighted ? (255 - controller.getColor().b) * 0.3f : 0);
					provinceStripes[it].color.r = owner.getColor().r + (Province.highlighted ? (255 - owner.getColor().r) * 0.3f : 0);
					provinceStripes[it].color.g = owner.getColor().g + (Province.highlighted ? (255 - owner.getColor().g) * 0.3f : 0);
					provinceStripes[it].color.b = owner.getColor().b + (Province.highlighted ? (255 - owner.getColor().b) * 0.3f : 0);
					provinceStripes[it].color.a = 255;
				}
			else for (auto it(provinces[provincesNeedingColorUpdate.front()].indexRange.first.first),
				end(provinces[provincesNeedingColorUpdate.front()].indexRange.first.second); it != end; ++it)
			{
				provinceFill[it].color.r = controller.getColor().r + (Province.highlighted ? (255 - controller.getColor().r) * 0.3f : 0);
				provinceFill[it].color.g = controller.getColor().g + (Province.highlighted ? (255 - controller.getColor().g) * 0.3f : 0);
				provinceFill[it].color.b = controller.getColor().b + (Province.highlighted ? (255 - controller.getColor().b) * 0.3f : 0);
				provinceStripes[it].color.a = 0;
			}

			//provincesLock.unlock();
			provincesNeedingColorUpdate.pop();
		}
		colorUpdateQueueLock.unlock();
		updatingColors = false;
		vertexArraysVisibilityNeedsUpdate = true;
	}
}
