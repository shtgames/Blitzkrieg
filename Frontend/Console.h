#pragma once

#include <mutex>

#include <GUI/TextField.h>
#include <GUI/Window.h>

namespace fEnd
{
	class Console final : public gui::Window
	{
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;
		void inputProcessor(const sf::String& input);
		
		mutable std::mutex m_outputLock;
		std::vector<sf::String> m_history;
		bool initialised = false, allowingInput = false;

		static const sf::Texture createTexture();

		static const sf::Texture m_background;
	public:
		Console();
		~Console() = default;

		const bool input(const sf::Event& event)override;

		void print(sf::String string);
		void eraseLastLine();
		void allowInput(const bool input);
		void init();
	};
}