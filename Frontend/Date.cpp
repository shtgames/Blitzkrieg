#include "Date.h"

#include "../Backend/TimeSystem.h"
#include "../Backend/Date.h"

#include <sstream>

namespace fEnd
{
	Date::Date(const Date& copy)
		: date(copy.date) {}

	Date::Date(const sf::Font& font)
	{
		date.setFont(font);
		date.setCharacterSize(13);
		date.setOutlineColor(sf::Color(15, 15, 15, 230));
		date.setFillColor(sf::Color(213, 213, 213));
	}

	std::unique_ptr<gui::Interactive> Date::copy() const
	{
		return std::unique_ptr<gui::Interactive>(new Date(*this));
	}

	std::unique_ptr<gui::Interactive> Date::move()
	{
		return std::unique_ptr<gui::Interactive>(new Date(std::move(*this)));
	}

	const bool Date::input(const sf::Event& event)
	{
		return false;
	}

	Date& fEnd::Date::setDate(const bEnd::Date& newDate)const
	{
		date.setString(toString(newDate));
		return (Date&)*this;
	}

	Date& Date::setFont(const sf::Font& font)
	{
		date.setFont(font);
		return *this;
	}

	Date& Date::setCharacterSize(const unsigned char characterSize)
	{
		date.setCharacterSize(characterSize);
		return *this;
	}

	Date& Date::setPosition(const float x, const float y)
	{
		date.setPosition(x, y);
		return *this;
	}

	Date& Date::setPosition(const sf::Vector2f& position)
	{ 
		return setPosition(position.x, position.y);
	}

	const sf::Vector2f& Date::getPosition() const
	{
		return date.getPosition();
	}

	const sf::FloatRect Date::getGlobalBounds() const
	{
		return date.getGlobalBounds();
	}

	const std::string Date::toString(const bEnd::Date& lVal)
	{
		std::string returnValue;

		returnValue += (lVal.getHour() < 10 ? "0" : "") + std::to_string(unsigned short(lVal.getHour())) +
			":00, " + std::to_string(unsigned short(lVal.getDay())) + " ";

		switch (lVal.getMonth())
		{
		case bEnd::January:
			returnValue += "January";
			break;
		case bEnd::February:
			returnValue += "February";
			break;
		case bEnd::March:
			returnValue += "March";
			break;
		case bEnd::April:
			returnValue += "April";
			break;
		case bEnd::May:
			returnValue += "May";
			break;
		case bEnd::June:
			returnValue += "June";
			break;
		case bEnd::July:
			returnValue += "July";
			break;
		case bEnd::August:
			returnValue += "August";
			break;
		case bEnd::September:
			returnValue += "September";
			break;
		case bEnd::October:
			returnValue += "October";
			break;
		case bEnd::November:
			returnValue += "November";
			break;
		case bEnd::December:
			returnValue += "December";
			break;
		}

		return returnValue += ", " + std::to_string(lVal.getYear());
	}

	void Date::draw(sf::RenderTarget& target, sf::RenderStates states)const
	{
		if (gui::Duration(gui::Internals::timeSinceStart() - timeOfLastUpdate) > gui::Duration(1.0f / gui::Internals::getUPS()))
		{
			setDate(bEnd::TimeSystem::getCurrentDate());
			timeOfLastUpdate = gui::Internals::timeSinceStart();
		}

		target.draw(date, states);
	}
}