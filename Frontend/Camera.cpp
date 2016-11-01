#include "Camera.h"

namespace fEnd
{
	const float Camera::lowerZoomLimitAsMapSizeFraction = 0.35f, Camera::upperZoomLimitAsMapSizeFraction = 0.025f;

	Camera::Camera(const Camera& copy)
		: view(copy.view), timeOfLastScroll(copy.timeOfLastScroll), scrollDirection(copy.scrollDirection), resolution(copy.resolution),
		position(copy.position), zoomFactor(copy.zoomFactor), scrollStep(copy.scrollStep) {}

	Camera::Camera(Camera&& temp) 
		: view(std::move(temp.view)), timeOfLastScroll(std::move(temp.timeOfLastScroll)), scrollDirection(temp.scrollDirection),
		resolution(temp.resolution), position(temp.position), zoomFactor(temp.zoomFactor), scrollStep(temp.scrollStep) {}

	Camera::Camera(const sf::Vector2s& resolution)
		: resolution(resolution)
	{
		view.reset(sf::FloatRect(position, sf::Vector2f(resolution)));
	}

	std::unique_ptr<gui::Interactive> Camera::copy() const
	{
		return std::unique_ptr<Interactive>(new Camera(*this));
	}

	std::unique_ptr<gui::Interactive> Camera::move()
	{
		return std::unique_ptr<Interactive>(new Camera(std::move(*this)));
	}

	const bool Camera::input(const sf::Event& event)
	{
		switch (event.type)
		{
		case sf::Event::MouseMoved:
			if (event.mouseMove.x == resolution.x - 1)
				scrollDirection.x = 1;
			else if (event.mouseMove.x == 0)
				scrollDirection.x = -1;
			else scrollDirection.x = 0;

			if (event.mouseMove.y == resolution.y - 1)
				scrollDirection.y = 1;
			else if (event.mouseMove.y == 0)
				scrollDirection.y = -1;
			else scrollDirection.y = 0;
			break;
		case sf::Event::MouseWheelMoved:
			zoom(event.mouseWheel.delta > 0 ? 1 / zoomFactor : zoomFactor, sf::Vector2f(event.mouseWheel.x, event.mouseWheel.y));
			break;
		default:
			return false;
		}
		return true;
	}

	Camera& Camera::setPosition(float x, float y)
	{
		viewLock.lock();
		if (x + view.getSize().x < 0) x += mapSize.x;
		if (x + view.getSize().x > mapSize.x) x -= mapSize.x;
		if (y + view.getSize().y / 2 < 0) y += mapSize.y;
		if (y + view.getSize().y / 2 > mapSize.y) y -= mapSize.y;

		view.setCenter(int(x + view.getSize().x / 2.0f), int(y + view.getSize().y / 2.0f));
		position.x = int(x);
		position.y = int(y);
		viewLock.unlock();
		hasChanged = true;

		return *this;
	}

	Camera& Camera::setSize(const sf::Vector2s& size)
	{
		viewLock.lock();
		view.setSize(size.x, size.y);
		viewLock.unlock();
		hasChanged = true;

		return *this;
	}

	Camera& Camera::setScreenResolution(const sf::Vector2s& newResolution)
	{
		resolution = newResolution;
		return *this;
	}

	Camera& Camera::setMapSize(const sf::Vector2s& newMapSize)
	{
		mapSize = newMapSize;
		if (mapSize.y * lowerZoomLimitAsMapSizeFraction < getSize().y)
			zoom(1.01f, sf::Vector2f(resolution.x / 2, resolution.y / 2));
		return *this;
	}

	const float Camera::getTotalZoom() const
	{
		return totalZoom;
	}

	const sf::Vector2f& Camera::getPosition() const
	{
		std::lock_guard<std::mutex> guard(viewLock);
		return position;
	}

	const sf::FloatRect Camera::getGlobalBounds() const
	{
		std::lock_guard<std::mutex> guard(viewLock);
		return sf::FloatRect(position, view.getSize());
	}

	const sf::Vector2f& Camera::getSize() const
	{
		std::lock_guard<std::mutex> guard(viewLock);
		return view.getSize();
	}

	const sf::Vector2f Camera::mapPixelToCoords(sf::Vector2f point) const
	{
		point.x *= totalZoom;
		point.y *= totalZoom;
		point.x += position.x;
		point.y += position.y;
		return point;
	}

	void Camera::draw(sf::RenderTarget& target, sf::RenderStates states) const 
	{
		std::lock_guard<std::mutex> guard(viewLock);
		scroll();
		target.setView(view);
	}

	void Camera::scroll()const
	{
		if ((scrollDirection.x != 0 || scrollDirection.y != 0) && gui::Duration(gui::Internals::timeSinceStart() - timeOfLastScroll) > gui::Duration(0.04))
		{			
			view.move(int(scrollDirection.x * (1 + scrollStep * totalZoom)), int(scrollDirection.y * (1 + scrollStep * totalZoom)));

			if (view.getCenter().x + (view.getSize().x / 2.0f) > mapSize.x)
				view.move(int(-mapSize.x), 0);
			else if (view.getCenter().x + (view.getSize().x / 2.0f) < 0)
				view.move(int(mapSize.x), 0);

			if (view.getCenter().y < 0)
				view.setCenter(view.getCenter().x, 0);
			else if (view.getCenter().y > mapSize.y)
				view.setCenter(view.getCenter().x, mapSize.y);

			position.x = int(view.getCenter().x - (view.getSize().x / 2.0f));
			position.y = int(view.getCenter().y - (view.getSize().y / 2.0f));
			
			hasChanged = true;
			timeOfLastScroll = gui::Internals::timeSinceStart();
		}
	}

	void Camera::zoom(float factor, sf::Vector2f targetPoint)
	{
		targetPoint.x = position.x + targetPoint.x * totalZoom;
		targetPoint.y = position.y + targetPoint.y * totalZoom;

		viewLock.lock();

		if ((view.getSize().y == mapSize.y * lowerZoomLimitAsMapSizeFraction && factor > 1.0f) ||
			(view.getSize().y == mapSize.y * upperZoomLimitAsMapSizeFraction && factor < 1.0f))
		{
			viewLock.unlock();
			return;
		}
		
		if (view.getSize().y * factor > mapSize.y * lowerZoomLimitAsMapSizeFraction)
			factor = mapSize.y * lowerZoomLimitAsMapSizeFraction / view.getSize().y;
		else if (view.getSize().y < mapSize.y * upperZoomLimitAsMapSizeFraction)
			factor = mapSize.y * upperZoomLimitAsMapSizeFraction / view.getSize().y;

		view.zoom(factor);
		view.move(int((targetPoint.x - view.getCenter().x) *  (1 - factor)),
			int((targetPoint.y - view.getCenter().y) *  (1 - factor)));

		if (view.getCenter().x + (view.getSize().x / 2.0f) > mapSize.x)
			view.move(-mapSize.x, 0);
		else if (view.getCenter().x + (view.getSize().x / 2.0f) < 0)
			view.move(mapSize.x, 0);

		if (view.getCenter().y < 0)
			view.setCenter(view.getCenter().x, 0);
		else if (view.getCenter().y > mapSize.y)
			view.setCenter(view.getCenter().x, mapSize.y);

		position.x = view.getCenter().x - view.getSize().x / 2;
		position.y = view.getCenter().y - view.getSize().y / 2;

		viewLock.unlock();

		totalZoom = factor * totalZoom;
		hasChanged = true;
	}
}
