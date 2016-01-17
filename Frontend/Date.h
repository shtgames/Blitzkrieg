#ifndef DATE_FRONTEND
#define DATE_FRONTEND

#include "..\..\Backend\Source Files\Date.h"

#include <SFML/Graphics.hpp>

namespace fEnd
{
	class Date final : public sf::Drawable
	{
	public:
		Date(const Date&) = default;
		Date(Date&&) = default;
		Date() = default;
		~Date() = default;
		
		Date& setDate(const bEnd::Date& date);
		Date& setFont(const sf::Font& font);
		Date& setCharacterSize(const unsigned char characterSize);
		Date& setPosition(const float x, const float y);
		Date& setPosition(const sf::Vector2f& position) { return setPosition(position.x, position.y); };

		static const std::string toString(const bEnd::Date&);
	private:
		void draw(sf::RenderTarget&, sf::RenderStates)const override;

		sf::Text date;
	};
}

#endif