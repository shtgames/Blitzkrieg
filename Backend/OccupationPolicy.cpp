#include "OccupationPolicy.h"

namespace bEnd
{
	void OccupationPolicy::setPolicy(const policies _policy)
	{
		if (_policy == CollaborationGovernment)
		{
			selectedPolicy = _policy;
			manpowerModifier = 0.35f;
			ICModifier = 0.05f;
			leadershipModifier = 0.3f;
			partisanActivityModifier = 1.5f;
			ResourceModifier = 0.25f;
		}
		else if (_policy == MilitaryGovernment)
		{
			selectedPolicy = _policy;
			manpowerModifier = 0.3f;
			ICModifier = 0.2f;
			leadershipModifier = 0.2f;
			partisanActivityModifier = 7.0f;
			ResourceModifier = 0.5f;
		}
		else if (_policy == FullOccupation)
		{
			selectedPolicy = _policy;
			manpowerModifier = 0.15f;
			ICModifier = 0.4f;
			leadershipModifier = 0.1f;
			partisanActivityModifier = 11.0f;
			ResourceModifier = 0.65f;
		}
		else if (_policy == TotalEconomicExploitation)
		{
			selectedPolicy = _policy;
			manpowerModifier = 0.05f;
			ICModifier = 0.6f;
			leadershipModifier = 0.01f;
			partisanActivityModifier = 19.0f;
			ResourceModifier = 0.75f;
		}
	}
}