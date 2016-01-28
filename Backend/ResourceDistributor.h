#ifndef RESOURCE_DISTRIBUTOR_BACKEND
#define RESOURCE_DISTRIBUTOR_BACKEND

#include "Resources.h"
#include "Tag.h"

#include <map>
#include <unordered_map>
#include <memory>

using namespace std;

namespace bEnd
{
	class Region;
	class ResourceDistributor final
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

		~ResourceDistributor() = default;

		const double getICDistributionValue(const ICDistributionCategory _Cval)const { return ICDistribution[_Cval].first * availableIC; }
		const float getResourceChange(const Resource _Rval, const ResourceChangeCategory _Cval)const { return resources[_Rval].second[_Cval]; };
		const float getResourceAmount(const Resource _Rval)const { return resources[_Rval].first; };
		const bool contains(const map<Resource, float>&)const;

		void transferResourcesFromTrade(const map<Resource, float>&);
		void transferResourcesFromRegion(const Region&);
		void transferManpowerFromRegion(const Region&);
		void transferICFromRegion(const Region&);
		void resetIncome();
		void update();
		void setICDistributionValue(const ICDistributionCategory category, const double factor);
		void setICDistributionValueLock(const ICDistributionCategory category, const bool lock = false);

		static const bool loadFromFile(ifstream&);
		static const bool exists(const Tag& tag) { if (resourceDistributors.count(tag) && resourceDistributors.at(tag)) return true; return false; }
		static ResourceDistributor& getResourceDistributor(const Tag& tag) { return *resourceDistributors.at(tag); }

	private:

		typedef map<ResourceChangeCategory, float>(ResourceChangeSummary);

		ResourceDistributor(const ResourceDistributor&) = default;
		ResourceDistributor(ResourceDistributor&&) = default;
		ResourceDistributor();

		void calculateIC();
		void resourceConversions();
		void distributeIC();
		void calculateMoney();
		
		float                                                        baseIC = BASE_IC, availableIC = BASE_IC, wastedIC = 0.0f, manpower = 0.0f;
		mutable map<Resource, pair<float, ResourceChangeSummary>>    resources;
		mutable map<ICDistributionCategory, std::pair<double, bool>> ICDistribution;
		mutable map<ICDistributionCategory, float>                   ICNeeds;
		const Tag                                                    tag;

		static unordered_map<Tag, unique_ptr<ResourceDistributor>> resourceDistributors;
		static const float BASE_IC;
	};
}

#endif