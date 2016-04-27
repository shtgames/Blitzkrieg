#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <GUI/WindowManager.h>
#include <GUI/Window.h>

namespace fEnd
{
	class Minimap final : public gui::Window
	{
		void draw(sf::RenderTarget& target, sf::RenderStates states);

		sf::VertexArray rectangle;
		sf::Sprite mapVisual;

	public:
		Minimap();
		~Minimap() = default;
	};

	class GameInterace final : public sf::Drawable
	{
	public:
		GameInterace();
		~GameInterace() = default;
		const bool input(const sf::Event& event);

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

		gui::WindowManager windows;
	};
}