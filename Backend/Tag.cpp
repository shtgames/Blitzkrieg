#include "Tag.h"

namespace bEnd
{
	Tag::Tag(const std::string& newTag)
	{
		if (newTag.size() >= 3)
		{
			a = newTag.at(0);
			b = newTag.at(1);
			c = newTag.at(2);
		}
	}

	Tag& Tag::operator=(const std::string& newTag)
	{
		if (newTag.size() >= 3)
		{
			a = newTag.at(0);
			b = newTag.at(1);
			c = newTag.at(2);
		}
		return *this;
	}

	const bool Tag::operator==(const Tag& tag)const
	{
		if (tag.a == a && tag.b == b && tag.c == c) return true;
		return false;
	}

	const bool Tag::operator==(const std::string& tag)const
	{
		if (tag.size() < 3) return false;
		if (tag.at(0) == a && tag.at(1) == b && tag.at(2) == c) return true;
		return false;
	}

	const bool Tag::operator!=(const Tag& tag)const
	{
		return !(this->operator==(tag));
	}

	const bool Tag::operator!=(const std::string& tag)const
	{
		return !(this->operator==(tag));
	}

	const bool Tag::operator>(const Tag& tag)const
	{
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
		returnValue.push_back(a);
		returnValue.push_back(b);
		returnValue.push_back(c);
		return returnValue;
	}

	const bool Tag::isTag(const std::string& tag)
	{
		return tag.size() >= 3 && tag.at(0) > 64 && tag.at(0) < 91 &&
			tag.at(1) > 64 && tag.at(1) < 91 && tag.at(2) > 64 && tag.at(2) < 91;
	}
}