#include "Tag.h"

namespace bEnd
{
	Tag::Tag(const std::string& _str)
	{
		if (_str.size() >= 3)
		{
			a = _str.at(0);
			b = _str.at(1);
			c = _str.at(2);
		}
		else
		{
			a = 0;
			b = 0;
			c = 0;
		}
	}

	Tag& Tag::operator=(const std::string& _str)
	{
		if (_str.size() >= 3)
		{
			lock.lock();
			a = _str.at(0);
			b = _str.at(1);
			c = _str.at(2);
			lock.unlock();
		}
		return *this;
	}

	const bool Tag::operator==(const Tag& tag)const
	{
		std::lock_guard<std::mutex> guard(lock), guard2(tag.lock);
		if (tag.a == a && tag.b == b && tag.c == c) return true;
		return false;
	}

	const bool Tag::operator==(const std::string& _str)const
	{
		if (_str.size() < 3) return false;
		std::lock_guard<std::mutex> guard(lock);
		if (_str.at(0) == a && _str.at(1) == b && _str.at(3) == c) return true;
		return false;
	}

	const bool Tag::operator!=(const Tag& _val)const
	{
		return !(this->operator==(_val));
	}

	const bool Tag::operator!=(const std::string& _str)const
	{
		return !(this->operator==(_str));
	}

	const bool Tag::operator>(const Tag& tag)const
	{
		std::lock_guard<std::mutex> guard(lock), guard2(tag.lock);

		if (a > tag.a) return true;
		if (tag.a > a) return false;
		if (b > tag.b) return true;
		if (tag.b > b) return false;
		if (c > tag.c) return true;
		if (tag.c > c) return false;
		return false;
	}

	const bool Tag::operator<(const Tag& tag)const
	{
		std::lock_guard<std::mutex> guard(lock), guard2(tag.lock);

		if (a < tag.a) return true;
		if (tag.a < a) return false;
		if (b < tag.b) return true;
		if (tag.b < b) return false;
		if (c < tag.c) return true;
		if (tag.c < c) return false;
		return false;
	}

	Tag::operator const std::string()const
	{
		std::string returnValue;
		lock.lock();
		returnValue.push_back(a);
		returnValue.push_back(b);
		returnValue.push_back(c);
		lock.unlock();
		return returnValue;
	}
}