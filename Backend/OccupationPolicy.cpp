#include "OccupationPolicy.h"

namespace bEnd
{
	const std::unordered_map<unsigned char, OccupationPolicy> OccupationPolicy::occupationPolicies {
		{ CollaborationGovernment, OccupationPolicy(CollaborationGovernment) },
		{ MilitaryGovernment, OccupationPolicy(MilitaryGovernment) },
		{ FullOccupation, OccupationPolicy(FullOccupation) },
		{ TotalEconomicExploitation, OccupationPolicy(TotalEconomicExploitation) }
	};

	void OccupationPolicy::setPolicy(const Policy newPolicy)
	{
		if (newPolicy == CollaborationGovernment)
		{
			manpowerModifier = 0.35f;
			ICModifier = 0.05f;
			leadershipModifier = 0.3f;
			partisanActivityModifier = 1.5f;
			resourceModifier = 0.25f;
		}
		else if (newPolicy == MilitaryGovernment)
		{
			manpowerModifier = 0.3f;
			ICModifier = 0.2f;
			leadershipModifier = 0.2f;
			partisanActivityModifier = 7.0f;
			resourceModifier = 0.5f;
		}
		else if (newPolicy == FullOccupation)
		{
			manpowerModifier = 0.15f;
			ICModifier = 0.4f;
			leadershipModifier = 0.1f;
			partisanActivityModifier = 11.0f;
			resourceModifier = 0.65f;
		}
		else if (newPolicy == TotalEconomicExploitation)
		{
			manpowerModifier = 0.05f;
			ICModifier = 0.6f;
			leadershipModifier = 0.01f;
			partisanActivityModifier = 19.0f;
			resourceModifier = 0.75f;
		}
	}

	/*const bool OccupationPolicy::populateOccupationPolicyMap()
	{
		occupationPolicies.emplace(std::make_pair(CollaborationGovernment, OccupationPolicy(CollaborationGovernment)));
		occupationPolicies.emplace(std::make_pair(MilitaryGovernment, OccupationPolicy(MilitaryGovernment)));
		occupationPolicies.emplace(std::make_pair(FullOccupation, OccupationPolicy(FullOccupation)));
		occupationPolicies.emplace(std::make_pair(TotalEconomicExploitation, OccupationPolicy(TotalEconomicExploitation)));
		return true;
	}*/
}
