#include "Console.h"

#include "../Frontend.hpp"

namespace fEnd
{
	constexpr auto width = 600, height = 400;

	const sf::Texture Console::m_background = Console::createTexture();

	void Console::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		std::lock_guard<std::mutex> guard(m_outputLock);
		Window::draw(target, states);
	}

	void Console::inputProcessor(const sf::String& output)
	{
		print(output);
	}

	const sf::Texture Console::createTexture()
	{
		sf::Texture returnValue;
		{
			sf::Image bg;
			bg.create(64, 64);
			for (size_t x(0); x != 64; ++x)
				for (size_t y(0); y != 64; ++y)
					bg.setPixel(x, y, sf::Color(10, 10, 10, 210));
			returnValue.loadFromImage(bg);
		}
		returnValue.setRepeated(true);

		return returnValue;
	}

	Console::Console()
	{
		setActive(false);
		setBackgroundTexture(m_background);
	}
	
	const bool Console::input(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Tilde)
		{
			setActive(!isActive());
			((gui::TextField&)at("input field")).setActive(isActive());
			return true;
		}
		return Window::input(event);
	}

	void Console::print(sf::String output)
	{
		if (output.isEmpty()) return;

		std::lock_guard<std::mutex> guard(m_outputLock);

		if (output.getSize() * ((gui::TextField&)at("input field")).getHeight() > width)
		{
			const size_t count(width / ((gui::TextField&)at("input field")).getHeight());
			if (count != 0)
				while (!output.isEmpty())
				{
					m_history.push_back(output.substring(0, count));
					if (output.getSize() < count) break;
					output = output.substring(count - 1);
				}
		}
		else m_history.push_back(output);

		sf::String visual;
		for (size_t i(0), end(m_history.size()); i != end; ++i)
			visual += m_history[i] + "\n";
		if (!visual.isEmpty()) visual.erase(visual.getSize() - 1);

		((gui::TextArea&)at("output field")).setText(visual);
		((gui::TextArea&)at("output field")).setPosition(5, height - ((gui::TextArea&)at("output field")).getGlobalBounds().height - ((gui::TextField&)at("input field")).getHeight() - 8);
	}

	void Console::init()
	{
		add("input field", gui::TextField(Resources::font("arial"), width - 25, 13)
			.setColor(sf::Color(220, 220, 220))
			.setCursorColor(sf::Color(245, 245, 245))
			.setInputProcessingFunction([&](const sf::String& input)
				{
					inputProcessor(input);
				})
			.setPosition(10, height - 2))
		.add("output field", gui::TextArea("", Resources::font("arial"), 13)
			.setColor(sf::Color(170, 170, 170)));

		setBackgroundTextureRect(sf::IntRect(0, 0, width, height + ((gui::TextField&)at("input field")).getHeight() + 5));
	}
}
