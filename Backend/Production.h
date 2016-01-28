#ifndef PRODUCTION_BACKEND
#define PRODUCTION_BACKEND

#include "Tag.h"
#include "Date.h"
#include "Resources.h"

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
		~Production() = default;
		
		void increaseProductionItemPriority(const unsigned short index);
		void decreaseProductionItemPriority(const unsigned short index);
		void setProductionItemAtMaxPriority(const unsigned short index);
		void setProductionItemAtMinPriority(const unsigned short index);
		void removeProductionItem(const unsigned short index);
		void addProductionItem(const string& element, const unsigned short targetRegion = -1);

		void deployUnit(const unsigned short index, const unsigned short targetRegion);		
		const float setIC(float IC);
		void update();
		
		static const bool loadFromFile(ifstream& file);
		static const bool exists(const Tag& tag) { if (production.count(tag) && production.at(tag)) return true; return false; }
		static Production& getProduction(const Tag& tag) { return *production.at(tag); }
		
	private:

		class ProductionItem
		{
		public:
			ProductionItem(const string& tech, const unsigned short days, const unsigned short region) : unit(tech), productionDays(days), targetRegion(region) {}
			ProductionItem(const ProductionItem&) = default;
			ProductionItem(ProductionItem&&) = default;
			ProductionItem() = delete;
			~ProductionItem() = default;

			ProductionItem& operator=(const ProductionItem&) = default;
			ProductionItem& operator=(ProductionItem&&) = default;

			const string& getUnit()const { return unit; }
			const double getCompletionPercentage()const { return completionPercentage; }
			const Date getComlpetionDate()const;
			const unsigned short getTargetRegion()const { return targetRegion; }

			ProductionItem& setIC(const float IC) { if (IC >= 0.0f) dedicatedICPercentage = IC <= 1.0f ? IC : 1.0f; return *this; }
			ProductionItem& setProductionDays(const unsigned short days) { productionDays = days; return *this; }

			const bool produce();
		private:
			const string unit;

			unsigned short productionDays = 0, targetRegion = -1;
			double completionPercentage = 0.0f;
			float dedicatedICPercentage = 0.0f;
		};

		Production(const Tag& tag) : tag(tag) {}
		Production(const Production&) = default;
		Production(Production&&) = default;
		Production() = default;

		const bool deploy(const ProductionItem& item, const unsigned short targetRegion);
		
		Production& setTag(const Tag& newTag) { tag = newTag; }

		vector<unique_ptr<ProductionItem>> productionLine;
		vector<unique_ptr<ProductionItem>> awaitingDeployment;
		float totalDedicatedIC = 0.0f;
		Tag tag;

		static unordered_map<Tag, unique_ptr<Production>> production;
	};
}

#endif