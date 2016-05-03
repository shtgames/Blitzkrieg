#pragma once

#include <GUI/Window.h>
#include <GUI/TextArea.h>

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

		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

		bEnd::Tag target;
		sf::Sprite shadow, flagShadow;
	};
}