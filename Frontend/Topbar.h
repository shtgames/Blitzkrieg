#pragma once

#include <GUI/Window.h>
#include <GUI/TextArea.h>
#include <GUI/Icon.h>

#include "../Backend/Tag.h"

namespace fEnd
{
	class Topbar final : public gui::Window
	{
	public:
		Topbar(const Topbar& copy) = default;
		Topbar(Topbar&& temp) = default;
		Topbar(const bEnd::Tag& tag);
		Topbar();
		~Topbar() = default;
		
		const bEnd::Tag& getTarget()const;
		Topbar& setTarget(const bEnd::Tag& tag);

		std::unique_ptr<gui::Window> copy()const override;
		std::unique_ptr<gui::Window> move()override;

	private:
		class GameSpeedIndicator final : public gui::Icon
		{
		public:
			GameSpeedIndicator(const GameSpeedIndicator& copy) = default;
			GameSpeedIndicator(GameSpeedIndicator&& temp) = default;
			GameSpeedIndicator();
			~GameSpeedIndicator() = default;
			
			std::unique_ptr<gui::Interactive> copy()const override { return std::unique_ptr<gui::Interactive>(new GameSpeedIndicator(*this)); }
			std::unique_ptr<gui::Interactive> move()override { return std::unique_ptr<gui::Interactive>(new GameSpeedIndicator(std::move(*this))); }

			GameSpeedIndicator& setPosition(const float x, const float y)override;

		private:
			void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

			gui::Icon paused;
		};

		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

		bEnd::Tag target;
		sf::Sprite shadow, flagShadow;
	};
}