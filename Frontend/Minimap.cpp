#include "Minimap.h"

#include "../Frontend.hpp"

#include "../Backend/TimeSystem.h"

namespace fEnd
{
	void Minimap::draw(sf::RenderTarget& target, sf::RenderStates states)const
	{
		if (gui::Duration(gui::Internals::timeSinceStart() - m_timeOfLastUpdate) > gui::Duration(1.0f / gui::Internals::getUPS()))
			update();

		target.setView(m_view);
		target.draw(m_map, states);

		target.draw(m_nightShadow, states);
		if (m_nightShadow.getPosition().x < m_map.getPosition().x)
		{
			const sf::Transform prev(states.transform);
			states.transform.translate(m_map.getTexture()->getSize().x, 0);
			target.draw(m_nightShadow, states);
			states.transform = prev;
		}

		target.draw(m_viewArea, states);
		if (m_viewArea.getPosition().x < m_map.getPosition().x)
		{
			const sf::Transform prev(states.transform);
			states.transform.translate(m_map.getTexture()->getSize().x, 0);
			target.draw(m_viewArea, states);
			states.transform = prev;
		}

		target.draw(background(), states);

		target.setView(target.getDefaultView());
	}

	void Minimap::update()const
	{
		const auto cameraBounds(Map::camera.getGlobalBounds());
		m_viewArea.setSize(sf::Vector2f((cameraBounds.width / Map::size().x) * m_map.getTexture()->getSize().x, 
			(cameraBounds.height / Map::size().y) * m_map.getTexture()->getSize().y));
		m_viewArea.setPosition(m_map.getPosition().x + (cameraBounds.left / Map::size().x) * m_map.getTexture()->getSize().x,
			m_map.getPosition().y + (cameraBounds.top / Map::size().y) * m_map.getTexture()->getSize().y);

		m_nightShadow.setPosition(m_map.getPosition().x - bEnd::TimeSystem::getCurrentDate().getHour() * m_map.getTexture()->getSize().x / 24,
			m_map.getPosition().y);

		m_timeOfLastUpdate = gui::Internals::timeSinceStart();
	}

	Minimap::Minimap(const sf::Vector2f& resolution)
		: Minimap()
	{
		setResolution(resolution);
	}

	Minimap::Minimap()
	{
		m_viewArea.setFillColor(sf::Color(0, 0, 0, 0));
		m_viewArea.setOutlineThickness(1);
		m_viewArea.setOutlineColor(sf::Color(200, 200, 200, 200));

		setBackgroundTexture(Resources::texture("minimap_frame"));
		m_map.setTexture(Resources::texture("minimap"));
		m_nightShadow.setTexture(Resources::texture("night_shadow"));
	}

	std::unique_ptr<gui::Window> Minimap::copy() const
	{
		return std::unique_ptr<gui::Window>(new Minimap(*this));
	}

	std::unique_ptr<gui::Window> Minimap::move()
	{
		return std::unique_ptr<gui::Window>(new Minimap(std::move(*this)));
	}

	const bool Minimap::input(const sf::Event& event)
	{
		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (m_map.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
			{
				if (event.mouseButton.button == sf::Mouse::Left)
					Map::camera.setPosition(((event.mouseButton.x - m_map.getPosition().x) / m_map.getTexture()->getSize().x) * Map::size().x - Map::camera.getSize().x / 2,
						((event.mouseButton.y - m_map.getPosition().y) / m_map.getTexture()->getSize().y) * Map::size().y - Map::camera.getSize().y / 2);
				return true;
			}
		}
		else if (event.type == sf::Event::MouseMoved && m_map.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				Map::camera.setPosition(((event.mouseMove.x - m_map.getPosition().x) / m_map.getTexture()->getSize().x) * Map::size().x - Map::camera.getSize().x / 2,
					((event.mouseMove.y - m_map.getPosition().y) / m_map.getTexture()->getSize().y) * Map::size().y - Map::camera.getSize().y / 2);
			return true;
		}
		return false;
	}

	void Minimap::setResolution(const sf::Vector2f& resolution)
	{
		setPosition(resolution.x - Resources::texture("minimap_frame").getSize().x, resolution.y - Resources::texture("minimap_frame").getSize().y);
		m_map.setPosition(resolution.x - Resources::texture("minimap").getSize().x - 3, resolution.y - Resources::texture("minimap").getSize().y - 11);

		m_view.setSize(sf::Vector2f(Resources::texture("minimap_frame").getSize()));
		m_view.setCenter(resolution.x - m_view.getSize().x / 2, resolution.y - m_view.getSize().y / 2);
		m_view.setViewport(sf::FloatRect((resolution.x - m_view.getSize().x) / resolution.x,
			(resolution.y - m_view.getSize().y) / resolution.y, m_view.getSize().x / resolution.x,
			m_view.getSize().y / resolution.y));
	}
}