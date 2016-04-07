#ifndef PRODUCTION_BACKEND
#define PRODUCTION_BACKEND

#include "Tag.h"
#include "Date.h"
#include "Resources.h"
#include "Unit.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <atomic>
#include <mutex>

using namespace std;

namespace bEnd
{
	class Region;
	class Production final
	{
	public:
		~Production() = default;

		void loadProductionItem(const FileProcessor::Statement& file);
		
		void increaseProductionItemPriority(const unsigned short index);
		void decreaseProductionItemPriority(const unsigned short index);
		void setProductionItemAtMaxPriority(const unsigned short index);
		void setProductionItemAtMinPriority(const unsigned short index);
		void removeProductionItem(const unsigned short index);
		void addProductionItem(const string& element, const unsigned short targetRegion = -1);

		void deployUnit(const unsigned short index, const unsigned short targetRegion);		
		const float setIC(float IC);
		void update();
		
		static const bool exists(const Tag& tag) { if (production.count(tag) && production.at(tag)) return true; return false; }
		static Production& get(const Tag& tag) { return *production.at(tag); }
		
	private:

		class ProductionItem
		{
		public:
			ProductionItem(const Unit& tech, const unsigned short days, const unsigned short region);
			ProductionItem(const ProductionItem&) = default;
			ProductionItem(ProductionItem&&) = default;
			ProductionItem() = delete;
			~ProductionItem() = default;
			
			const Unit& getUnit()const { return unit; }
			const double getCompletionPercentage()const { return completionPercentage; }
			const Date getComlpetionDate()const;
			const unsigned short getTarget()const { return targetRegion; }

			ProductionItem& setIC(const float IC) { if (IC >= 0.0f) dedicatedICPercentage = IC <= 1.0f ? IC : 1.0f; return *this; }
			ProductionItem& updateProductionDays(const Tag& tag);

			const bool produce();
		private:
			const Unit& unit;

			atomic<unsigned short> productionDays = 0, targetRegion = -1;
			atomic<float> completionPercentage = 0.0f;
			atomic<float> dedicatedICPercentage = 0.0f;
		};

		Production(const Tag& tag) : tag(tag) {}
		Production(const Production&) = default;
		Production(Production&&) = default;
		Production() = delete;

		const bool deploy(const ProductionItem& item, const unsigned short targetRegion);
		
		vector<unique_ptr<ProductionItem>> productionLine;
		vector<unique_ptr<ProductionItem>> deploymentQueue;
		atomic<float> totalDedicatedIC = 0.0f;
		const Tag tag;
		mutex productionLineLock, deploymentQueueLock;

		static unordered_map<Tag, unique_ptr<Production>> production;
	};
}

#endif