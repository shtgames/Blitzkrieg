#pragma once

#include <mutex>
#include <atomic>

#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <GUI/Interactive.h>
#include <GUI/Internals.h>

#include "utilities.h"

namespace fEnd
{
	class Camera final : public gui::Interactive
	{
	public:
		Camera(const Camera& copy);
		Camera(Camera&& temp);
		Camera(const sf::Vector2s& resolution);
		Camera() = default;
		~Camera() = default;

		std::unique_ptr<Interactive> copy()const override;
		std::unique_ptr<Interactive> move()override;

		const bool input(const sf::Event& event) override;

		Camera& setPosition(const float x, const float y)override;
		Camera& setSize(const sf::Vector2s& size);
		Camera& setScreenResolution(const sf::Vector2s& resolution);
		Camera& setMapSize(const sf::Vector2s& mapSize);

		const sf::Vector2f& getPosition()const override;
		const sf::FloatRect getGlobalBounds()const override;

		mutable std::atomic<bool> hasChanged = false;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

		void scroll()const;
		void zoom(float factor, sf::Vector2f targetPoint);

		mutable sf::View view;
		mutable gui::TimePoint timeOfLastScroll;
		sf::Vector2s scrollDirection = sf::Vector2s(0, 0), resolution = sf::Vector2s(1024, 768), mapSize;
		mutable sf::Vector2f position = sf::Vector2f(0, 0);
		float zoomFactor = 1.10f, scrollStep = 15;
		mutable std::mutex viewLock;
	};
}