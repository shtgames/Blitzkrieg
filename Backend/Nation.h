#ifndef NATION_BACKEND
#define NATION_BACKEND

#include "Tag.h"
#include "Production.h"
#include "Leadership.h"
#include "Politics.h"
#include "Intelligence.h"

#include <unordered_map>
#include <set>

namespace bEnd
{
	class Nation final
	{
	public:

		Nation(const Nation&) = default;
		Nation(Nation&&) = default;
		~Nation() = default;
		Nation() = default;

		Nation& operator=(const Nation&) = default;
		Nation& operator=(Nation&&) = default;

		Production   production;
		Politics     politics;
		Intelligence intelligence;
		Leadership   technology;

		static void updateGlobal();
		static void loadNations();
		static std::map<Tag, Nation> nations;

	private:
		enum Continent
		{
			Europe,
			Asia,
			Africa,
			NorthAmerica,
			SouthAmerica
		};

		Continent   continent = Europe;
		std::string name = "";
		Tag         tag = "NUL";
		bool        major = false;

		static Tag player;

		static void resetIncomeGlobal();
	};
}
#endif