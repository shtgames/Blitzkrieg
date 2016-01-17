#include "Date.h"

#include <sstream>

const std::string DATE_FORMAT = "HH, DD MM, YYYY";

namespace fEnd
{
	Date& fEnd::Date::setDate(const bEnd::Date& newDate)
	{
		date.setString(toString(newDate));
		return *this;
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

	const std::string Date::toString(const bEnd::Date& lVal)
	{
		std::string returnValue;
		for (auto it = DATE_FORMAT.begin(), end = DATE_FORMAT.end(); it != end; ++it)
		{
			if (*it == 'H')
			{
				returnValue += lVal.getHour() < 10 ? "0" : "";
				returnValue += static_cast<std::ostringstream*>(&(std::ostringstream() << lVal.getHour()))->str() + ":00";
				it++;
				continue;
			}
			else if (*it == 'D')
			{
				returnValue += static_cast<std::ostringstream*>(&(std::ostringstream() << lVal.getDay()))->str();
				it++;
				continue;
			}
			else if (*it == 'M')
			{
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
				it++;
				continue;
			}
			else if (*it == 'Y')
			{
				returnValue += static_cast<std::ostringstream*>(&(std::ostringstream() << lVal.getYear()))->str();
				it++; it++; it++;
				continue;
			}
			else returnValue += *it;
		}
		return returnValue;
	}
	void Date::draw(sf::RenderTarget& target, sf::RenderStates states)const
	{
		target.draw(date, states);
	}
}