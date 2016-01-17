#include "Leadership.h"

#include "OccupationPolicy.h"
#include "Region.h"

namespace bEnd
{
	void Leadership::update()
	{
		distributeLeadership();
		research.update();
		espionage.update();
	}

	void Leadership::resetLeadership()
	{
		baseLeadership = 0;
		availableLeadership = 0;
		wastedLeadership = 0;
	}

	void Leadership::transferLeadership(const Region& region, const OccupationPolicy& occupationPolicy)
	{
		if (region.hasCore(region.getController()) && region.getController() == region.getOwner())
			baseLeadership += region.getLeadershipGeneration();
		else if (region.getController() == region.getOwner())
			baseLeadership += region.getLeadershipGeneration() * 0.5f;
		else
			baseLeadership += region.getLeadershipGeneration() * occupationPolicy.getLeadershipModifier();
	}

	void Leadership::distributeLeadership()
	{
		wastedLeadership += research.setLeadership(availableLeadership * leadershipDistribution[ToResearch]);
		wastedLeadership += research.setLeadership(availableLeadership * leadershipDistribution[ToEspionage]);
	}
}