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
	class Province;
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
		void addProductionItem(const string& element, const unsigned short targetProvince = -1);

		void deployUnit(const unsigned short index, const unsigned short targetProvince);		
		const float setIC(float IC);
		void update();
		void reset();
		
		static const bool exists(const Tag& tag) { if (production.count(tag) && production.at(tag)) return true; return false; }
		static void emplace(const Tag& tag) { production[tag].reset(new Production(tag)); }
		static Production& get(const Tag& tag) { if (!production.count(tag)) emplace(tag); return *production.at(tag); }
		
	private:
		class ProductionItem
		{
		public:
			ProductionItem(const Unit& tech, const unsigned short days, const unsigned short Province);
			ProductionItem(const ProductionItem&) = default;
			ProductionItem(ProductionItem&&) = default;
			ProductionItem() = delete;
			~ProductionItem() = default;
			
			const Unit& getUnit()const { return unit; }
			const double getCompletionPercentage()const { return completionPercentage; }
			const Date getComlpetionDate()const;
			const unsigned short getTarget()const { return targetProvince; }

			ProductionItem& setIC(const float IC) { if (IC >= 0.0f) dedicatedICPercentage = (IC <= 1.0f ? IC : 1.0f); return *this; }
			ProductionItem& updateProductionDays(const Tag& tag);

			const bool produce();
		private:
			const Unit& unit;

			atomic<unsigned short> productionDays = {0}, targetProvince = {(unsigned short)-1};
			atomic<float> completionPercentage = {0.0f};
			atomic<float> dedicatedICPercentage = {0.0f};
		};

		Production(const Tag& tag) : tag(tag) {}
		Production(const Production&) = default;
		Production(Production&&) = default;
		Production() = delete;

		const bool deploy(const ProductionItem& item, const unsigned short targetProvince);
		
		vector<unique_ptr<ProductionItem>> productionLine;
		vector<unique_ptr<ProductionItem>> deploymentQueue;
		atomic<float> totalDedicatedIC = {0.0f};
		const Tag tag;
		mutex productionLineLock, deploymentQueueLock;

		static unordered_map<Tag, unique_ptr<Production>> production;
	};
}

#endif
