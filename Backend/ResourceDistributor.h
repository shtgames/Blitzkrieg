#ifndef RESOURCE_DISTRIBUTOR_BACKEND
#define RESOURCE_DISTRIBUTOR_BACKEND

#include "Resources.h"
#include "Tag.h"
#include "FileProcessor.h"

#include <map>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>

using namespace std;

namespace bEnd
{
	class Region;
	class ResourceDistributor final
	{
		friend class TimeSystem;
	public:
		enum ICDistributionCategory
		{
			ToUpgrades,
			ToReinforcement,
			ToSupplyProduction,
			ToProductionLine,
			ToConsumerGoods,
			ToLendLease
		};

		enum ResourceChangeCategory
		{
			Generated,
			Used,
			Traded,
			ConvertedTo,
			ConvertedFrom,
			IntoNetwork,
			ReturnedToStockpile,
			Last,
			Total
		};

		~ResourceDistributor() = default;

		void loadFromSave(const FileProcessor::Statement&);

		const float getDistributedICAmount(const ICDistributionCategory category)const { lock_guard<mutex> guard(ICDistributionLock); return ICDistribution[category].first * IC.first * IC.second * ICResourceBottleneck; }
		void setICDistributionValue(const ICDistributionCategory category, const double factor);
		void setICDistributionValueLock(const ICDistributionCategory category, const bool lock = false);

		const float getICAmount()const { return IC.first * IC.second; }
		const float getResourceGain(const Resource resource, const ResourceChangeCategory category)const 
		{ 
			lock_guard<mutex> guard(resourcesLock);
			return resources[resource].second[category].first * resources[resource].second[category].second;
		}
		const float getResourceAmount(const Resource resource)const { lock_guard<mutex> guard(resourcesLock); return resources[resource].first; };
		const float getManpowerGain()const { return manpower.second.first * manpower.second.second; }
		const float getManpowerAmount()const { return manpower.first; }
		
		void changeICAmount(const float amount) { IC.first = IC.first + amount; }
		void changeManpowerGain(const float amount) { manpower.second.first = manpower.second.first + amount; }
		void changeManpowerAmount(const float amount) { manpower.first + amount >= 0.0f ? manpower.first = manpower.first + amount : manpower.first = 0.0f; }
		void changeResourceGain(const Resource resource, const float changeAmount, const ResourceChangeCategory category)
		{
			lock_guard<mutex> guard(resourcesLock);
			resources[resource].second[category].first += changeAmount;
		}
		void changeResourceAmount(const Resource resource, const float amount)
		{ 
			lock_guard<mutex> guard(resourcesLock);
			resources[resource].first + amount >= 0.0f ? resources[resource].first += amount : resources[resource].first = 0.0f;
		}
		
		const bool contains(const map<Resource, float>& resources)const;
		void update();

		static const bool exists(const Tag& tag) { if (resourceDistributors.count(tag) && resourceDistributors.at(tag)) return true; return false; }
		static void emplace(const Tag& tag) { resourceDistributors[tag].reset(new ResourceDistributor(tag)); }
		static ResourceDistributor& get(const Tag& tag) { if (!resourceDistributors.count(tag)) emplace(tag); return *resourceDistributors.at(tag); }

	private:
		typedef map<ResourceChangeCategory, pair<float, float>>(ResourceChangeSummary);

		ResourceDistributor(const ResourceDistributor&) = default;
		ResourceDistributor(ResourceDistributor&&) = default;
		ResourceDistributor() = delete;
		ResourceDistributor(const Tag& tag);

		void calculateICResourceBottleneck();
		void resourceConversions();
		void distributeIC();
		void calculateMoneyChangeAmount();

		atomic<float>                                             wastedIC = 0.0f, ICResourceBottleneck = 1.0f;
		pair<atomic<float>, atomic<float>>                        IC = make_pair(BASE_IC, 1.0f);
		pair<atomic<float>, pair<atomic<float>, atomic<float>>>   manpower;
		mutable map<Resource, pair<float, ResourceChangeSummary>> resources;

		mutable map<ICDistributionCategory, std::pair<float, bool>> ICDistribution;

		const Tag tag;

		mutable mutex resourcesLock, ICDistributionLock;

		static unordered_map<Tag, unique_ptr<ResourceDistributor>> resourceDistributors;
		static const float BASE_IC, ENERGY_PER_IC_POINT, METAL_PER_IC_POINT, RARE_MATERIALS_PER_IC_POINT;
	};
}

#endif