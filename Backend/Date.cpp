#include "Date.h"

namespace bEnd
{
	const Date Date::NEVER = Date(0, 1, January, unsigned short(-1));

	Month& operator++(Month& lVal)
	{
		switch (lVal)
		{
		case January:
			lVal = February;
			break;
		case February:
			lVal = March;
			break;
		case March:
			lVal = April;
			break;
		case April:
			lVal = May;
			break;
		case May:
			lVal = June;
			break;
		case June:
			lVal = July;
			break;
		case July:
			lVal = August;
			break;
		case August:
			lVal = September;
			break;
		case September:
			lVal = October;
			break;
		case October:
			lVal = November;
			break;
		case November:
			lVal = December;
			break;
		case December:
			lVal = January;
			break;
		}
		return lVal;
	}

	Date::Date(const unsigned char _hour, const unsigned char _day, const Month _month, const unsigned short _year)
		: year(_year != 0 ? _year : 1), month(_month != 0 && _month <= 12 ? _month : January), hour(_hour <= 24 ? _hour : 0)
	{
		if (day != 0 && ((day <= 28 && month == 2 && !isLeapYear()) ||
			(day <= 29 && month == 2 && isLeapYear()) ||
			(day <= 30 && (month == 4 || month == 6 || month == 9 || month == 11)) ||
			(day <= 31 && (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12))))
			day = _day;
		else day = 1;
	}

	Date::Date(const unsigned long long _hours)
	{
		unsigned long long days = _hours / 24;

		year = 400 * (days / ((400.0f * 365.0f) + 97.0f)) + 1;
		year += 100 * ((days % (400 * 365) + 97) / ((100 * 365) + 24)) + ((days % (400 * 365) + 97) < (100 * 365) + 24 ? 1 : 0);

		days -= (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400;
		const unsigned short months[13] = { 0, 31, isLeapYear() ? 60 : 59, isLeapYear() ? 91 : 90, isLeapYear() ? 121 : 120, isLeapYear() ? 152 : 151, isLeapYear() ? 182 : 181, isLeapYear() ? 213 : 212, isLeapYear() ? 244 : 243, isLeapYear() ? 274 : 273, isLeapYear() ? 305 : 304, isLeapYear() ? 335 : 334, isLeapYear() ? 366 : 365 };
		for (month = January; month < 13, days > months[month]; month++);

		day = days - months[month - 1];

		hour = _hours % 24;
	}

	Date::Date() : year(1), month(January), day(1), hour(0) {}

	Date Date::operator+(const unsigned short days)const
	{
		Date returnValue;
		return returnValue;
	}

	Date Date::operator-(const unsigned short days)const
	{
		Date returnValue;
		return returnValue;
	}

	Date Date::operator+(const Date& _Date)const
	{
		Date returnValue;
		return returnValue;
	}

	Date Date::operator-(const Date& _Date)const
	{
		Date returnValue;
		return returnValue;
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

	const bool Date::operator<(const Date& _Date)const
	{
		if (year < _Date.year) return true;
		else if (year > _Date.year) return false;

		if (month < _Date.month) return true;
		else if (month > _Date.month) return false;

		if (day < _Date.day) return true;
		else if (day > _Date.day) return false;

		if (hour < _Date.hour) return true;
		else return false;
	}

	const bool Date::operator==(const Date& _Date)const
	{
		if (hour == _Date.hour && day == _Date.day && month == _Date.month && year == _Date.year) return true;
		return false;
	}

	const bool Date::isLeapYear()const
	{
		if (year % 400 == 0) return true;
		if (year % 4 == 0 && year % 100 != 0) return true;
		return false;
	}
}
