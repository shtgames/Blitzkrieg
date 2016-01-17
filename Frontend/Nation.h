#ifndef NATION_FRONTEND
#define NATION_FRONTEND

#include <memory>
#include <SFML/Graphics.hpp>

namespace fEnd
{
	class Nation final
	{
	public:
		Nation(const sf::Texture& flag, const sf::Color& color) : flag(new sf::Texture(flag)), color(color) {}
		Nation(const Nation&) = default;
		Nation(Nation&&) = default;
		~Nation() = default;
		Nation() = default;

		const std::shared_ptr<const sf::Texture> getFlag()const { return flag; }
		const sf::Color& getColor()const { return color; }

		Nation& setFlag(const sf::Texture& texture) { flag.reset(new sf::Texture(texture)); return *this; }
		Nation& setColor(const sf::Color& newColor) { color = newColor; return *this; }
	private:
		std::shared_ptr<const sf::Texture> flag = nullptr;
		sf::Color color = sf::Color(0, 0, 0);
	};
}

#endif