#ifndef GAME_RESOURCES
#define GAME_RESOURCES

#include <unordered_map>

namespace bEnd
{
	enum Resource
	{
		Energy,
		Metal,
		CrudeOil,
		RareMaterials,
		Supplies,
		Fuel,
		Money,
		Last
	};
}

namespace std
{
	template <> struct hash<bEnd::Resource>
	{
		size_t operator()(const bEnd::Resource& x) const
		{
			return size_t (x);
		}
	};
}

#endif
