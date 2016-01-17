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
			a = _str.at(0);
			b = _str.at(1);
			c = _str.at(2);
		}
		return *this;
	}

	const bool Tag::operator==(const Tag& _val)const
	{
		if (_val.getA() == a && _val.getB() == b && _val.getC() == c) return true;
		return false;
	}

	const bool Tag::operator==(const std::string& _str)const
	{
		if (_str.size() < 3) return false;
		if (_str.at(0) == a && _str.at(1) == b && _str.at(3) == c) return true;
		return false;
	}

	const bool Tag::operator!=(const Tag& _val)const
	{
		if (_val.getA() != a || _val.getB() != b || _val.getC() != c) return true;
		return false;
	}

	const bool Tag::operator!=(const std::string& _str)const
	{
		if (_str.size() < 3) return false;
		if (_str.at(0) != a || _str.at(1) != b || _str.at(3) != c) return true;
		return false;
	}

	const bool Tag::operator>(const Tag& _Tag)const
	{
		if (a > _Tag.getA()) return true;
		if (_Tag.getA() > a) return false;
		if (b > _Tag.getA()) return true;
		if (_Tag.getA() > b) return false;
		if (c > _Tag.getA()) return true;
		if (_Tag.getA() > c) return false;
		return false;
	}

	const bool Tag::operator<(const Tag& _Tag)const
	{
		if (a < _Tag.getA()) return true;
		if (_Tag.getA() < a) return false;
		if (b < _Tag.getA()) return true;
		if (_Tag.getA() < b) return false;
		if (c < _Tag.getA()) return true;
		if (_Tag.getA() < c) return false;
		return false;
	}

	Tag::operator const std::string()const
	{
		std::string returnValue;
		returnValue.push_back(a);
		returnValue.push_back(b);
		returnValue.push_back(c);
		return returnValue;
	}
}