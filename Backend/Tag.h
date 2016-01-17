#ifndef GAME_TAG
#define GAME_TAG

#include <unordered_map>

namespace bEnd
{
	class Tag final
	{
	public:
		Tag(const std::string&);
		Tag(const Tag&) = default;
		Tag(Tag&&) = default;
		~Tag() = default;
		Tag() : a(0), b(0), c(0) {}

		Tag& operator=(const Tag&) = default;
		Tag& operator=(Tag&&) = default;

		Tag& operator=(const std::string&);
		const bool operator==(const Tag&)const;
		const bool operator==(const std::string&)const;
		const bool operator!=(const Tag&)const;
		const bool operator!=(const std::string&)const;
		const bool operator>(const Tag&)const;
		const bool operator<(const Tag&)const;

		operator const std::string()const;

		const char& getA()const { return a; }
		const char& getB()const { return b; }
		const char& getC()const { return c; }

	private:
		char a = 0, b = 0, c = 0;
	};
}

namespace std
{
	template<> struct hash<bEnd::Tag>
	{
		size_t operator()(const bEnd::Tag& val) const
		{
			return size_t(0) | (hash<char>()(val.getA()) | (hash<char>()(val.getB()) << sizeof(char)) | (hash<char>()(val.getC()) << 2 * sizeof(char)));
		}
	};

	template<> struct hash<std::pair<bool, bool>>
	{
		size_t operator()(const std::pair<bool, bool>& val) const
		{
			return hash<bool>()(val.first) | (hash<bool>()(val.second) << sizeof(bool));
		}
	};

	template<> struct hash<std::pair<bEnd::Tag, bEnd::Tag>>
	{
		size_t operator()(const std::pair<bEnd::Tag, bEnd::Tag>& val) const
		{
			return hash<bEnd::Tag>()(val.first) | (hash<bEnd::Tag>()(val.second) << 3 * sizeof(char));
		}
	};

	template <> struct less<bEnd::Tag>
	{
		bool operator()(const bEnd::Tag& lVal, const bEnd::Tag& rVal) const
		{
			return lVal < rVal;
		}
	};

	template <> struct less<std::pair<bool, bool>>
	{
		bool operator()(const std::pair<bool, bool>& lVal, const std::pair<bool, bool>& rVal) const
		{
			return lVal.first < rVal.first ? true : (lVal.second < rVal.second ? true : false);
		}
	};

	template <> struct less<std::pair<bEnd::Tag, bEnd::Tag>>
	{
		bool operator()(const std::pair<bEnd::Tag, bEnd::Tag>& lVal, const std::pair<bEnd::Tag, bEnd::Tag>& rVal) const
		{
			return lVal.first < rVal.first ? true : lVal.second < rVal.second ? true : false;
		}
	};
};

#endif