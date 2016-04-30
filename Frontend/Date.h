#ifndef DATE_FRONTEND
#define DATE_FRONTEND

#include <SFML/Graphics.hpp>

#include <GUI/Internals.h>
#include <GUI/Interactive.h>

namespace bEnd
{
	class Date;
}

namespace fEnd
{
	class Date final : public gui::Interactive
	{
	public:
		Date(const Date&);
		Date(Date&&) = default;
		Date(const sf::Font& font);
		Date() = default;
		~Date() = default;
		
		std::unique_ptr<gui::Interactive> copy()const override;
		std::unique_ptr<gui::Interactive> move()override;

		const bool input(const sf::Event& event)override;

		Date& setDate(const bEnd::Date& date)const;
		Date& setFont(const sf::Font& font);
		Date& setCharacterSize(const unsigned char characterSize);
		Date& setPosition(const float x, const float y);
		Date& setPosition(const sf::Vector2f& position)override;

		const sf::Vector2f& getPosition()const override;
		const sf::FloatRect getGlobalBounds()const override;

		static const std::string toString(const bEnd::Date&);

	private:
		void draw(sf::RenderTarget&, sf::RenderStates)const override;

		mutable sf::Text date;
		mutable gui::TimePoint timeOfLastUpdate;
	};
}

#endif