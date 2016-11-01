#ifndef OCCUPATION_POLICY_BACKEND
#define OCCUPATION_POLICY_BACKEND

#include <string>
#include <unordered_map>

namespace bEnd
{
	enum Policy
	{
		TotalEconomicExploitation,
		FullOccupation,
		MilitaryGovernment,
		CollaborationGovernment
	};

	class OccupationPolicy final
	{
	public:
		OccupationPolicy(const Policy policy) { setPolicy(policy); };
		OccupationPolicy() = default;

		const float getMPModifier()const { return manpowerModifier; }
		const float getICModifier()const { return ICModifier; }
		const float getLeadershipModifier()const { return leadershipModifier; }
		const float getPartisanActivityModifier()const { return partisanActivityModifier; }
		const float getResourceModifier()const { return resourceModifier; }

		static const OccupationPolicy& get(const Policy policy) { return occupationPolicies.at(policy); };
		
	private:
		void setPolicy(const Policy);

		float manpowerModifier, ICModifier, leadershipModifier, partisanActivityModifier, resourceModifier = 0;

		static const std::unordered_map<unsigned char, OccupationPolicy> occupationPolicies;

	};
}

#endif
