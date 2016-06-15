#pragma once

#include <GUI/WindowManager.h>
#include <SFML/Graphics/RenderWindow.hpp>

namespace fEnd
{
	class GameInterface;
	class NationSelectScreen final
	{
		gui::WindowManager main;
	public:
		NationSelectScreen(const sf::Vector2u& resolution);

		void run(sf::RenderWindow& target, GameInterface& gameInterface);
	};
}