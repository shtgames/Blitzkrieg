#include "Region.h"

#include "LeadershipDistributor.h"

#include "Research.h"
#include "Espionage.h"

namespace bEnd
{
	unordered_map<Tag, LeadershipDistributor> LeadershipDistributor::leadershipDistributor;
	const float                               LeadershipDistributor::BASE_LEADERSHIP = 2.0f;

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

	const bool LeadershipDistributor::loadFromFile(ifstream& file)
	{
		return false;
	}

	void LeadershipDistributor::distributeLeadership()
	{
		wastedLeadership += bEnd::Research::getResearch(tag).setLeadership(availableLeadership * leadershipDistribution[ToResearch]);
		// wastedLeadership += bEnd::Espionage::
	}
};