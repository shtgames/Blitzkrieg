#pragma once

#include <GUI/Window.h>

namespace fEnd
{
	class ProvincePanel final : public gui::Window
	{
	public:
		ProvincePanel(const sf::Vector2u& resolution);
		ProvincePanel(const ProvincePanel& copy) = default;
		ProvincePanel(ProvincePanel&& copy) = default;

		std::unique_ptr<gui::Window> copy()const override;
		std::unique_ptr<gui::Window> move()override;

		const bool input(const sf::Event& event)override;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;
		void setParent(const gui::WindowManager* const parent)override;
	};
}