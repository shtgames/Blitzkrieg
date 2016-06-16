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

		class ResourceCell final
		{
		public:
			struct Mod final
			{
				const float get()const
				{
					return add * mult;
				}
				float add = 0, mult = 1;
			};
			void resetTotal()
				{
					for (auto it(1); it != ResourceChangeCategory::Last; ++it)
						m_mod[(ResourceChangeCategory)it].add = 0;
					m_mod[Total].add = get(Generated);
				}
			void add(const ResourceChangeCategory cat, const float amount, const bool reset = false)
			{
				if (cat == Total) return;
				m_mod[Total].add -= m_mod[cat].get();
				reset ? m_mod[cat].add = amount : m_mod[cat].add += amount;
				m_mod[Total].add += m_mod[cat].get();
			}
			void mult(const ResourceChangeCategory cat, const float amount, const bool reset = false)
				{
					if (cat == Total) return;
					reset ? m_mod[cat].mult = amount: m_mod[cat].mult *= amount;
				}
			const float add(const ResourceChangeCategory cat)
				{
					return m_mod[cat].add;
				}
			const float mult(const ResourceChangeCategory cat)
				{
					return m_mod[cat].mult;
				}
			const float get(const ResourceChangeCategory cat)
			{
				return m_mod[cat].get();
			}
			const auto& operator[] (const ResourceChangeCategory cat)
			{
				return m_mod[cat];
			}
			const auto& at(const ResourceChangeCategory cat)
			{
				return m_mod.at(cat);
			}
			void apply()
			{
				amount += m_mod[Total].add;
			}

			float amount = 0;
		private:
			std::unordered_map<ResourceChangeCategory, Mod> m_mod;
		};

		~ResourceDistributor() = default;

		const float getDistributedICAmount(const ICDistributionCategory category)const 
		{
			lock_guard<mutex> guard(ICDistributionLock); 
			return ICDistribution[category].first * IC.first * IC.second * ICResourceBottleneck; 
		}
		void setICDistributionValue(const ICDistributionCategory category, const double factor);
		void setICDistributionValueLock(const ICDistributionCategory category, const bool lock = false);

		const float getAvailableIC()const { return IC.first * IC.second; }
		const float getBaseIC()const { return IC.first; }
		const float getICResourceBottleneck()const { return ICResourceBottleneck; }
		const float getWastedIC()const { return wastedIC; }
		const float getResourceGain(const Resource resource, const ResourceChangeCategory category)const 
		{
			lock_guard<mutex> guard(resourcesLock);
			return resources[resource][category].get();
		}
		const float getResourceAmount(const Resource resource)const { lock_guard<mutex> guard(resourcesLock); return resources[resource].amount; };
		const float getManpowerGain()const { return manpower.second.first * manpower.second.second; }
		const float getManpowerAmount()const { return manpower.first; }
		
		void changeICAmount(const float amount) { IC.first = IC.first + amount; }
		void changeManpowerGain(const float amount) { manpower.second.first = manpower.second.first + amount; }
		void changeManpowerAmount(const float amount) { manpower.first + amount >= 0.0f ? manpower.first = manpower.first + amount : manpower.first = 0.0f; }
		void changeResourceGain(const Resource resource, const float changeAmount, const ResourceChangeCategory category)
		{
			if (category == Total) return;
			lock_guard<mutex> guard(resourcesLock);
			resources[resource].add(category, changeAmount);
		}
		void changeResourceAmount(const Resource resource, const float amount)
		{
			lock_guard<mutex> guard(resourcesLock);
			resources[resource].amount + amount >= 0.0f ? resources[resource].amount += amount : resources[resource].amount = 0.0f;
		}
		
		const bool contains(const map<Resource, float>& resources)const;
		void update();

		static void reset();
		static const bool exists(const Tag& tag) { if (resourceDistributors.count(tag) && resourceDistributors.at(tag)) return true; return false; }
		static void emplace(const Tag& tag) { resourceDistributors[tag].reset(new ResourceDistributor(tag)); }
		static ResourceDistributor& get(const Tag& tag) { if (!resourceDistributors.count(tag)) emplace(tag); return *resourceDistributors.at(tag); }

	private:
		typedef map<ResourceChangeCategory, pair<float, float>>(ResourceChangeSummary);

		ResourceDistributor(const ResourceDistributor&) = default;
		ResourceDistributor(ResourceDistributor&&) = default;
		ResourceDistributor() = delete;
		ResourceDistributor(const Tag& tag);

		void useResourcesForIC();
		void resourceConversions();
		void distributeIC();
		void calculateMoneyChangeAmount();

		atomic<float>                                           wastedIC = 0.0f, ICResourceBottleneck = 1.0f;
		pair<atomic<float>, atomic<float>>                      IC = make_pair(BASE_IC, 1.0f);
		pair<atomic<float>, pair<atomic<float>, atomic<float>>> manpower;
		mutable map<Resource, ResourceCell>                     resources;

		mutable map<ICDistributionCategory, std::pair<float, bool>> ICDistribution;

		const Tag tag;

		mutable mutex resourcesLock, ICDistributionLock;

		static unordered_map<Tag, unique_ptr<ResourceDistributor>> resourceDistributors;
		static const float BASE_IC, ENERGY_PER_IC_POINT, METAL_PER_IC_POINT, RARE_MATERIALS_PER_IC_POINT;
	};
}

#endif