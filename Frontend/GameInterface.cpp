#include "GameInterface.h"

#include "../Frontend.hpp"
#include "../Backend/Nation.h"

#include <GUI/FPSMeter.h>
#include <GUI/Button.h>

namespace fEnd
{
	GameInterface::GameInterface(const sf::Vector2u& resolution)
	{
		Map::updateRegionVisuals(sf::Vector2s(resolution.x, resolution.y));

		cursor.setTexture(Resources::texture("cursor"));

		m_windows
			.emplace("Topbar", Topbar(), true)
			.emplace("Minimap", Minimap(sf::Vector2f(resolution)), true)
			.emplace("Map", gui::Window().add("Map", fEnd::Map()), true);

		m_fpsMeter.setFont(Resources::font("arial")).setCharacterSize(17).setColor(sf::Color::White).setPosition(20, 70);
	}

	void GameInterface::input(const sf::Event& event)
	{
		if (event.type == sf::Event::Closed) std::exit(0);
		if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::R && event.key.control)
		{
			m_showFPSMeter = !m_showFPSMeter;
			return;
		}
		else if (event.type == sf::Event::MouseMoved)
		{
			cursor.setPosition(event.mouseMove.x, event.mouseMove.y);
			m_windows.at("Map", true).input(event);
		}

		m_windows.input(event);
	}

	void GameInterface::resetResolution(const sf::Vector2u& resolution)
	{
		Map::updateRegionVisuals(sf::Vector2s(resolution.x, resolution.y));
	}

	void GameInterface::updatePlayer()
	{
		((Topbar&)m_windows.at("Topbar", true)).setTarget(bEnd::Nation::player);
	}

	void GameInterface::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(m_windows);
		if (m_showFPSMeter) target.draw(m_fpsMeter);

		target.draw(cursor);
	}
}