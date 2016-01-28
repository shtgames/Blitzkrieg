#ifndef LEADERSHIP_DISTRIBUTOR_BACKEND
#define LEADERSHIP_DISTRIBUTOR_BACKEND

#include "Tag.h"

#include <fstream>
#include <unordered_map>
#include <map>
#include <memory>

using namespace std;

namespace bEnd
{
	class Region;
	class LeadershipDistributor final
	{
	public:
		enum LeadershipDistributionCategory
		{
			ToResearch,
			ToEspionage,
			ToDiplomacy,
			ToOfficers
		};

		~LeadershipDistributor() = default;

		const float getLeadershipDistributionValue(const LeadershipDistributionCategory category)const { return leadershipDistribution[category].first * availableLeadership; }

		void update();

		void resetLeadership();
		void transferLeadership(const Region& region);
		void setICDistributionValue(const LeadershipDistributionCategory category, const double factor);
		void setICDistributionValueLock(const LeadershipDistributionCategory category, const bool lock = false);

		static const bool loadFromFile(ifstream&);
		static const bool exists(const Tag& tag) { if (leadershipDistributor.count(tag) && leadershipDistributor.at(tag)) return true; return false; }
		static LeadershipDistributor& getLeadershipDistributor(const Tag& tag) { return *leadershipDistributor.at(tag); }

	private:

		LeadershipDistributor(const LeadershipDistributor&) = default;
		LeadershipDistributor(LeadershipDistributor&&) = default;
		LeadershipDistributor();

		void distributeLeadership();

		float                                                                baseLeadership = 2.0f, availableLeadership = 2.0f, wastedLeadership = 0.0f;
		mutable map<LeadershipDistributionCategory, std::pair<double, bool>> leadershipDistribution;
		const Tag                                                            tag;

		static unordered_map<Tag, unique_ptr<LeadershipDistributor>> leadershipDistributor;
		static const float BASE_LEADERSHIP;
	};
}
#endif
