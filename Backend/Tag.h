#ifndef GAME_TAG
#define GAME_TAG

#include <atomic>
#include <string>

namespace bEnd
{
	class Tag;
}

namespace std
{
	template<> struct hash<bEnd::Tag>;
}

namespace bEnd
{
	class Tag final
	{
	public:
		Tag(const std::string&);
		Tag(const Tag& tag) : a(char(tag.a)), b(char(tag.b)), c(char(tag.c)) {};
		Tag(Tag&&) = default;
		~Tag() = default;
		Tag() : a('N'), b('U'), c('L') {}

		Tag& operator=(const Tag& tag) { a = char(tag.a); b = char(tag.b); c = char(tag.c); return *this; };
		Tag& operator=(Tag&&) = default;

		Tag& operator=(const std::string&);
		const bool operator==(const Tag&)const;
		const bool operator==(const std::string&)const;
		const bool operator!=(const Tag&)const;
		const bool operator!=(const std::string&)const;
		const bool operator>(const Tag&)const;
		const bool operator<(const Tag&)const;

		operator const std::string()const;

		static const bool isTag(const std::string& tag);

	private:
		std::atomic<char> a, b, c;

		friend struct std::hash<Tag>;
	};
}

namespace std
{
	template<> struct hash<bEnd::Tag>
	{
		size_t operator()(const bEnd::Tag& val) const
		{
			return size_t(0) | (hash<char>()(val.a) | (hash<char>()(val.b) << sizeof(char)) | (hash<char>()(val.c) << 2 * sizeof(char)));
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