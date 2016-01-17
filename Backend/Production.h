#ifndef PRODUCTION_BACKEND
#define PRODUCTION_BACKEND

#include "Resources.h"
#include "Date.h"
#include "OccupationPolicy.h"
#include "ProductionItem.h"

#include <map>
#include <vector>

namespace bEnd
{
	class Production final
	{
	public:
		enum ICDistributionCategory
		{
			Upgrades,
			Reinforcement,
			SupplyProduction,
			ProductionLine,
			ConsumerGoods,
			LendLease
		};

		enum ResourceChangeCategory
		{
			generated,
			used,
			traded,
			convertedIntoCrudeOil,
			convertedFromEnergy,
			convertedIntoFuel,
			convertedFromCrudeOil,
			intoNetwork,
			returnedToStockpile
		};
		
		Production(const Production&) = default;
		Production(Production&&) = default;
		Production();

		Production& operator=(const Production&) = default;
		Production& operator=(Production&&) = default;
		
		const float& getICDistributionValue(const ICDistributionCategory& _Cval)const { return ICDistribution.at(_Cval) * availableIC; }
		const float& getResourceChange(const Resource& _Rval, const ResourceChangeCategory& _Cval)const { return resources.at(_Rval).second.at(_Cval); };
		const float& getResourceAmount(const Resource& _Rval)const { return resources.at(_Rval).first; };
		const bool contains(const std::map<Resource, float>&)const;
		void transferResourcesFromTrade(const std::map<Resource, float>&);
		void resetIncome();
		void update();

	private:

		typedef std::map<ResourceChangeCategory, float>(ResourceChangeSummary);

		void transferResourcesFromRegion(const std::map<Resource, float>&, const bool&, const bool&, const OccupationPolicy&);
		void transferManpowerFromRegion(const float&, const bool&, const bool&, const OccupationPolicy&);
		void transferIC(const unsigned short&, const bool&, const bool&, const OccupationPolicy&);

		void calculateIC();
		void resourceConversions();

		void distributeIC();
		void ICToUpgrades();
		void ICToReinforcement();
		void ICToSupplies();
		void ICToProductionLine();
		void ICToConsumerGoods();
		void ICToLendLease();
		void calculateMoney();

		void increaseProductionItemPriority(const unsigned short&);
		void decreaseProductionItemPriority(const unsigned short&);
		void setProductionItemAtMaxPriority(const unsigned short&);
		void setProductionItemAtMinPriority(const unsigned short&);
		void removeProductionItem(const unsigned short&);
		void addProductionItem(ProductionItem*);

		float baseIC = 0.0f, availableIC = 0.0f, wastedIC = 0.0f, manpower = 0.0f;
		std::map<Resource, std::pair<float, ResourceChangeSummary>> resources;
		std::map<ICDistributionCategory, float> ICDistribution, ICNeeds;

		std::vector<ProductionItem*> productionLine;

		friend class Region;
		friend class Diplomacy;
	};
}

#endif