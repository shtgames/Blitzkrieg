#include "Region.h"

#include "LeadershipDistributor.h"

#include "Research.h"
#include "Espionage.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<LeadershipDistributor>> LeadershipDistributor::leadershipDistributor;

	const float LeadershipDistributor::BASE_LEADERSHIP = 2.0f;

	LeadershipDistributor::LeadershipDistributor()
		: baseLeadership(2.0f), availableLeadership(2.0f), wastedLeadership(0.0f)
	{
		leadershipDistribution[ToResearch].first = 1.0f;
	}

	void LeadershipDistributor::update()
	{
		distributeLeadership();
		bEnd::Research::getResearch(tag).update();
		//bEnd::Espionage::
	}

	void LeadershipDistributor::resetLeadership()
	{
		baseLeadership = BASE_LEADERSHIP;
		availableLeadership = BASE_LEADERSHIP; // * bonuses;
		wastedLeadership = 0;
	}

	void LeadershipDistributor::transferLeadership(const Region& region)
	{
		baseLeadership += region.getLeadershipGeneration() ;
		availableLeadership += region.getLeadershipGeneration(); // * bonuses;
	}

	void LeadershipDistributor::setICDistributionValue(const LeadershipDistributionCategory category, const double factor)
	{
		leadershipDistribution[category].second = false;
		const double difference = (factor >= 0.0f ? factor : 0.0f) - leadershipDistribution[category].first;

		if (difference > 0.0f) do
		{
			double min = 1.0f;
			unsigned char unlockedCategoryCount = 0;

			for (auto it = leadershipDistribution.begin(), end = leadershipDistribution.end(); it != end; ++it)
			{
				if (it->first == category) continue;
				if (!it->second.second && !(difference > 0.0f && it->second.first == 0.0f))
				{
					unlockedCategoryCount++;
					if (it->second.first < min) min = it->second.first;
				}
			}
			if (unlockedCategoryCount == 0) break;

			const double changeAmount = difference / unlockedCategoryCount > min ? min : difference / unlockedCategoryCount;
			for (auto it = leadershipDistribution.begin(), end = leadershipDistribution.end(); it != end; ++it)
				if (it->first != category) it->second.first -= changeAmount;

			leadershipDistribution[category].first += changeAmount * unlockedCategoryCount;

		} while (leadershipDistribution[category].first != factor);
		else
		{
			unsigned char unlockedCategoryCount = 0;
			for (auto it = leadershipDistribution.begin(), end = leadershipDistribution.end(); it != end; ++it)
			{
				if (it->first == category) continue;
				if (!it->second.second) unlockedCategoryCount++;
			}

			const double changeAmount = difference / unlockedCategoryCount;
			for (auto it = leadershipDistribution.begin(), end = leadershipDistribution.end(); it != end; ++it)
				if (it->first != category) it->second.first -= changeAmount;

			leadershipDistribution[category].first += changeAmount * unlockedCategoryCount;
		}
	}

	void LeadershipDistributor::setICDistributionValueLock(const LeadershipDistributionCategory category, const bool lock)
	{
		leadershipDistribution[category].second = lock;
	}

	const bool LeadershipDistributor::loadFromFile(ifstream& file)
	{
		return false;
	}

	void LeadershipDistributor::distributeLeadership()
	{
		wastedLeadership += bEnd::Research::getResearch(tag).setLeadership(availableLeadership * leadershipDistribution[ToResearch].first);
		// wastedLeadership += bEnd::Espionage::
	}
};