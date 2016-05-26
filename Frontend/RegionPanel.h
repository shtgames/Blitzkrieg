#pragma once

#include <GUI/Window.h>

namespace fEnd
{
	class RegionPanel final : public gui::Window
	{
	public:
		RegionPanel(const sf::Vector2u& resolution);
		RegionPanel(const RegionPanel& copy) = default;
		RegionPanel(RegionPanel&& copy) = default;

		std::unique_ptr<gui::Window> copy()const override;
		std::unique_ptr<gui::Window> move()override;

		const bool input(const sf::Event& event)override;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;
	};
}