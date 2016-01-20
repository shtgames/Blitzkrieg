#ifndef PRODUCTION_BACKEND
#define PRODUCTION_BACKEND

#include "Tag.h"
#include "Date.h"
#include "Resources.h"
#include "ProductionItem.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <fstream>

using namespace std;

namespace bEnd
{
	class Region;
	class Production final
	{
	public:

		void increaseProductionItemPriority(const unsigned short);
		void decreaseProductionItemPriority(const unsigned short);
		void setProductionItemAtMaxPriority(const unsigned short);
		void setProductionItemAtMinPriority(const unsigned short);
		void removeProductionItem(const unsigned short);
		void addProductionItem(std::unique_ptr<ProductionItem>&& element);
		
		const float setIC(const float IC);
		void update();
		
		static const bool loadFromFile(ifstream& file);

		static Production& getProduction(const Tag& tag) { return production[tag]; }
		
	private:
		
		Production(const Production&) = default;
		Production(Production&&) = default;
		Production() = default;
		~Production() = default;
				
		vector<std::unique_ptr<ProductionItem>> productionLine;

		static unordered_map<Tag, Production> production;
	};
}

#endif