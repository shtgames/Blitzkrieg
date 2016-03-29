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

		Date(const Date& copy) = default;
		Date(Date&& temp) = default;
		Date() = default;
		~Date() = default;
		Date(const unsigned char hour, const unsigned char day, const Month month, const unsigned short year);
		Date(const unsigned long long hours);

		Date& operator=(const Date& copy) = default;
		Date& operator=(Date&& temp) = default;

		explicit operator unsigned long long()const;

		Date operator+(const unsigned long long hours)const;
		Date operator-(const unsigned long long hours)const;
		Date operator+(const Date& amount)const;
		Date operator-(const Date& amount)const;

		Date& operator++();
		const bool operator<(const Date& date)const;
		const bool operator>(const Date& date)const;
		const bool operator==(const Date& date)const;
		const bool operator!=(const Date& date)const;

		const unsigned short getYear()const;
		const unsigned char getDay()const;
		const unsigned char getMonth()const;
		const unsigned char getHour()const;

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