#ifndef NATION_BACKEND
#define NATION_BACKEND

#include "Region.h"
#include "OccupationPolicy.h"
#include "Tag.h"

#include <unordered_map>
#include <set>

using namespace std;

namespace bEnd
{
	class Nation final
	{
	public:

		Nation(const Nation&) = default;
		Nation(Nation&&) = default;
		Nation() = default;
		~Nation() = default;

		static void updateGlobal();
		static void loadNations();
		static map<Tag, Nation> nations;

	private:

		enum Continent
		{
			Europe,
			Asia,
			Africa,
			NorthAmerica,
			SouthAmerica
		};

		Continent continent = Europe;
		string    name = "";
		Tag       tag = "NUL";
		bool      major = false;

		static Tag player;
	};
}
#endif