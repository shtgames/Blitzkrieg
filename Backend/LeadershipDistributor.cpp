#include "Region.h"
#include "Tech.h"

#include "LeadershipDistributor.h"

#include "Research.h"
#include "Espionage.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<LeadershipDistributor>> LeadershipDistributor::leadershipDistributor;

	const float LeadershipDistributor::BASE_LEADERSHIP = 2.0f;

	LeadershipDistributor::LeadershipDistributor(const Tag& tag)
		: tag(tag)
	{
		for (auto it(0); it <= LeadershipDistributionCategory::ToOfficers; it++)
			leadershipDistribution[(LeadershipDistributionCategory)it] = std::make_pair(0.0f, false);
		leadershipDistribution[ToResearch] = std::make_pair(1.0f, false);
	}

	void LeadershipDistributor::loadFromSave(const FileProcessor::Statement& source)
	{
		if (source.lValue != "leadership") return;

		setLeadershipDistributionValue(ToResearch, std::stof(source.rStrings.at(0)));
		setLeadershipDistributionValueLock(ToResearch, true);
		setLeadershipDistributionValue(ToEspionage, std::stof(source.rStrings.at(1)));
		setLeadershipDistributionValueLock(ToEspionage, true);
		setLeadershipDistributionValue(ToDiplomacy, std::stof(source.rStrings.at(2)));
		setLeadershipDistributionValueLock(ToDiplomacy, true);

		setLeadershipDistributionValueLock(ToResearch);
		setLeadershipDistributionValueLock(ToEspionage);
		setLeadershipDistributionValueLock(ToDiplomacy);
	}

	void LeadershipDistributor::update()
	{
		distributeLeadership();

		Research::get(tag).update();
		//bEnd::Espionage::
		//bEnd::...
	}

	void LeadershipDistributor::reset()
	{
		Research::get(tag).reset();

		leadershipDistributionLock.lock();
		for (auto it(0); it <= LeadershipDistributionCategory::ToOfficers; it++)
			leadershipDistribution[(LeadershipDistributionCategory)it] = std::make_pair(0.0f, false);
		leadershipDistribution[ToResearch] = std::make_pair(1.0f, false);
		leadershipDistributionLock.unlock();

		wastedLeadership = 0;
		leadership = make_pair(BASE_LEADERSHIP, 1.0f);
	}

	void LeadershipDistributor::setLeadershipDistributionValue(const LeadershipDistributionCategory category, const double factor)
	{
		leadershipDistributionLock.lock();
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
		leadershipDistributionLock.unlock();
	}

	void LeadershipDistributor::setLeadershipDistributionValueLock(const LeadershipDistributionCategory category, const bool lock)
	{
		leadershipDistributionLock.lock();
		leadershipDistribution[category].second = lock;
		leadershipDistributionLock.unlock();
	}

	void LeadershipDistributor::distributeLeadership()
	{
		leadershipDistributionLock.lock();
		wastedLeadership = wastedLeadership + bEnd::Research::get(tag).setLeadership(leadership.first * leadership.second * leadershipDistribution[ToResearch].first);
		// wastedLeadership += bEnd::Espionage::
		leadershipDistributionLock.unlock();
	}
};