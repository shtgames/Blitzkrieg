#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <GUI/WindowManager.h>
#include <GUI/Window.h>
#include <GUI/FPSMeter.h>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace fEnd
{
	void setIcon(sf::RenderWindow& window);

	class GameInterface final : public sf::Drawable
	{
	public:
		GameInterface(const sf::Vector2u& resolution);
		~GameInterface() = default;

		void input(const sf::Event& event);

		void resetResolution(const sf::Vector2u& resolution);
		void updatePlayer();

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

		gui::WindowManager m_windows;
		sf::Sprite cursor;
		gui::FPSMeter m_fpsMeter;
		bool m_showFPSMeter = false;
	};
}