#ifndef LEADERSHIP_BACKEND
#define LEADERSHIP_BACKEND

#include "Tag.h"

#include <fstream>

using namespace std;

namespace bEnd
{
	class Region;
	class LeadershipDistributor final
	{
	public:
		enum leadershipDistributionCategories
		{
			ToResearch,
			ToEspionage,
			ToDiplomacy,
			ToOfficers
		};
		
		const float getLeadershipDistributionValue(const leadershipDistributionCategories category)const { return leadershipDistribution[category] * availableLeadership; }

		void update();

		void resetLeadership();
		void transferLeadership(const Region& region);

		static const bool loadFromFile(ifstream&);

		static LeadershipDistributor& getLeadershipDistributor(const Tag& tag) { return leadershipDistributor[tag]; }

	private:
		
		Leadership(const Leadership&) = default;
		Leadership(Leadership&&) = default;
		Leadership() = default;
		~Leadership() = default;

		Leadership& operator=(const Leadership&) = default;
		Leadership& operator=(Leadership&&) = default;

		void distributeLeadership();

		float                                                baseLeadership = 2.0f, availableLeadership = 0.0f, wastedLeadership = 0.0f;
		mutable map<leadershipDistributionCategories, float> leadershipDistribution;
		const Tag                                            tag;

		static unordered_map<Tag, LeadershipDistributor> leadershipDistributor;
		static const float                               BASE_LEADERSHIP;
}

#endif
