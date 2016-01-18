#ifndef LEADERSHIP_BACKEND
#define LEADERSHIP_BACKEND

#include "Research.h"
#include "Espionage.h"

namespace bEnd
{
	class Region;
	class OccupationPolicy;
	class Leadership final
	{
	public:
		enum leadershipDistributionCategories
		{
			ToResearch,
			ToEspionage,
			ToDiplomacy,
			ToOfficers
		};

		Leadership(const Leadership&) = default;
		Leadership(Leadership&&) = default;
		Leadership() = default;
		~Leadership() = default;

		Leadership& operator=(const Leadership&) = default;
		Leadership& operator=(Leadership&&) = default;

		const float getLeadershipDistributionValue(const leadershipDistributionCategories category)const { return leadershipDistribution[category] * availableLeadership; };

		void update();

		void resetLeadership();
		void transferLeadership(const Region& region, const OccupationPolicy& occupationPolicy);

	private:
		void distributeLeadership();

		float                                                     baseLeadership = 0.0f, availableLeadership = 0.0f, wastedLeadership = 0.0f;
		mutable std::map<leadershipDistributionCategories, float> leadershipDistribution;

		Research research;
		Espionage espionage;
}

#endif
