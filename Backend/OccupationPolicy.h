#ifndef OCCUPATION_POLICY_BACKEND
#define OCCUPATION_POLICY_BACKEND

#include <string>

namespace bEnd
{
	class OccupationPolicy final
	{
	public:
		enum policies
		{
			CollaborationGovernment,
			MilitaryGovernment,
			FullOccupation,
			TotalEconomicExploitation
		};

		OccupationPolicy() { setPolicy(TotalEconomicExploitation); };

		const policies getPolicy()const { return selectedPolicy; }
		const float getMPModifier()const { return manpowerModifier; }
		const float getICModifier()const { return ICModifier; }
		const float getLeadershipModifier()const { return leadershipModifier; }
		const float getPartisanActivityModifier()const { return partisanActivityModifier; }
		const float getResourceModifier()const { return ResourceModifier; }

		void setPolicy(const policies);
	private:
		std::string name, description;
		policies    selectedPolicy;
		float       manpowerModifier, ICModifier, leadershipModifier, partisanActivityModifier, ResourceModifier;
	};
}

#endif