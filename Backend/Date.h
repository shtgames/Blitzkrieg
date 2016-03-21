#ifndef DATE_BACKEND
#define DATE_BACKEND

#include <string>
#include <atomic>

namespace bEnd
{
	enum Month
	{
		January = 1,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};
	
	class Date final
	{
	public:

		Date(const unsigned char hour, const unsigned char day, const Month month, const unsigned short year);
		Date(const unsigned long long);
		Date(const Date& copy);
		Date(Date&&) = default;
		Date();
		~Date() = default;

		explicit operator unsigned long long()const;

		Date& operator=(const Date&);
		Date& operator=(Date&&) = default;

		Date operator+(const unsigned long long hours)const;
		Date operator-(const unsigned long long hours)const;
		Date operator+(const Date& amount)const;
		Date operator-(const Date& amount)const;

		Date& operator++();
		const bool operator<(const Date& date)const;
		const bool operator>(const Date& date)const;
		const bool operator==(const Date& date)const;
		const bool operator!=(const Date& date)const;

		const unsigned short getYear()const { return year; }
		const unsigned char getDay()const { return day; }
		const unsigned char getMonth()const { return month; }
		const unsigned char getHour()const { return hour; }

		static const Date NEVER;

	private:

		const bool isLeapYear()const;

		std::atomic<unsigned char> day = 1, hour = 0;
		std::atomic<unsigned char> month = January;
		std::atomic<unsigned short> year = 1;

		static const unsigned short monthToDays[13];

		friend class TimeSystem;
	};
}


#endif