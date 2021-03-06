#include "Date.h"

namespace bEnd
{
	typedef unsigned char (uChar);
	typedef unsigned short (uShort);
	typedef unsigned long long (uLongLong);

	const Date Date::NEVER = Date(24, 31, December, uChar(-1));
	const unsigned short Date::monthToDays[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

	Date::Date(const Date& copy)
		: day(uChar(copy.day)), hour(uChar(copy.hour)), month(uChar(copy.month)), year(uShort(copy.year))
	{}

	Date::Date(const uChar newHour, const uChar newDay, const Month newMonth, const unsigned short newYear)
		: hour(newHour <= 24 ? newHour : 0), month(newMonth != 0 && newMonth <= 12 ? newMonth : January), year(newYear != 0 ? newYear : 1)
	{
		setDay(newDay);
	}

	Date::Date(const unsigned long long hours)
	{
		unsigned long long days = hours / 24;

		year = 400 * (days / ((400.0f * 365.0f) + 97.0f)) + 1;
		year += 100 * ((days % (400 * 365) + 97) / ((100 * 365) + 24)) + ((days % (400 * 365) + 97) < (100 * 365) + 24 ? 1 : 0);

		days -= (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400;
		
		for (month = January; month < 13, days > monthToDays[month] +
			(month >= 2 ? (isLeapYear() ? 1 : 0) : 0); month++);
		day = days - monthToDays[month - 1];

		hour = hours  % 24;
	}

	Date& Date::operator=(const Date& copy)
	{
		day = uChar(copy.day);
		hour = (uChar)(copy.hour);
		month = (uChar)(copy.month);
		year = (uShort)(copy.year);

		return *this;
	}

	Date::operator unsigned long long() const
	{
		return (year * 365 + uShort(year * (97.0f / 400.0f)) +
			monthToDays[month - 1] + (month - 1 >= 2 ? (isLeapYear() ? 1 : 0) : 0) +
			day) * 24 + hour;
	}

	Date Date::operator+(const unsigned long long hours)const
	{
		return Date((*this).operator unsigned long long int() + hours);
	}

	Date Date::operator-(const unsigned long long hours)const
	{
		return Date((*this).operator unsigned long long int() - hours);
	}

	Date Date::operator+(const Date& date)const
	{
		return Date((*this).operator unsigned long long int() + (date).operator unsigned long long int());
	}

	Date Date::operator-(const Date& date)const
	{
		return Date((*this).operator unsigned long long int() + (date).operator unsigned long long int());
	}

	Date& Date::operator++()
	{
		hour++;
		if (hour == 24)
		{
			hour = 0;
			day++;
			if ((day == 29 && month == 2 && !isLeapYear()) ||
				(day == 30 && month == 2 && isLeapYear()) ||
				(day == 31 && (month == 4 || month == 6 || month == 9 || month == 11)) ||
				(day == 32 && (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)))
			{
				day = 1;
				month++;
				if (month == 13)
				{
					year++;
					month = January;
				}
			}
		}
		return *this;
	}

	const bool Date::operator<(const Date& date)const
	{
		if (year < date.year) return true;
		else if (year > date.year) return false;

		if (month < date.month) return true;
		else if (month > date.month) return false;

		if (day < date.day) return true;
		else if (day > date.day) return false;

		if (hour < date.hour) return true;
		else return false;
	}

	const bool Date::operator>(const Date& date) const
	{
		return !this->operator<(date) && !this->operator==(date);
	}

	const bool Date::operator==(const Date& date)const
	{
		return hour == date.hour && day == date.day && month == date.month && year == date.year;
	}

	const bool Date::operator!=(const Date& date) const
	{
		return !operator==(date);
	}

	Date& Date::setHour(const unsigned char newHour)
	{
		if (newHour <= 24) hour = newHour;
		return *this;
	}

	Date& Date::setDay(const unsigned char newDay)
	{
		if (day != 0 && ((day <= 28 && month == 2 && !isLeapYear()) ||
			(day <= 29 && month == 2 && isLeapYear()) ||
			(day <= 30 && (month == 4 || month == 6 || month == 9 || month == 11)) ||
			(day <= 31 && (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12))))
			day = newDay;

		return *this;
	}

	Date& Date::setMonth(const unsigned char newMonth)
	{
		if (newMonth <= 12) month = newMonth;
		return *this;
	}

	Date& Date::setYear(const unsigned short newYear)
	{
		if (newYear != 0) year = newYear;
		return *this;
	}

	const unsigned short Date::getYear() const
	{
		return year;
	}

	const unsigned char Date::getDay() const
	{
		return day;
	}

	const unsigned char Date::getMonth() const
	{
		return month;
	}

	const unsigned char Date::getHour() const
	{
		return hour;
	}

	const bool Date::isLeapYear()const
	{
		return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
	}
}
