#pragma once

#include <GUI/Internals.h>
#include <GUI/Window.h>

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <unordered_map>

namespace fEnd
{
	class Minimap final : public gui::Window
	{
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;
		void update()const;

		mutable gui::TimePoint m_timeOfLastUpdate;
		mutable sf::RectangleShape m_viewArea;
		mutable sf::Sprite m_map, m_nightShadow;
		mutable sf::View m_view;

	public:
		Minimap(const sf::Vector2f& resolution);
		Minimap(const Minimap& copy) = default;
		Minimap(Minimap&& temp) = default;
		Minimap();
		~Minimap() = default;

		std::unique_ptr<gui::Window> copy()const override;
		std::unique_ptr<gui::Window> move()override;

		const bool input(const sf::Event& event)override;

		void setResolution(const sf::Vector2f& resolution);
	};

}