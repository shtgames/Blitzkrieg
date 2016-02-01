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

		const float getLeadershipDistributionAmount(const LeadershipDistributionCategory category)const { return leadershipDistribution[category].first * leadership.first * leadership.second; }

		void setLeadershipDistributionValue(const LeadershipDistributionCategory category, const double factor);
		void setLeadershipDistributionValueLock(const LeadershipDistributionCategory category, const bool lock = false);

		const float getLeadershipAmount()const { return leadership.first * leadership.second; }
		void changeLeadershipAmount(const float amount) { leadership.first + amount >= 0.0f ? leadership.first + amount : leadership.first = 0.0f; }

		void update();

		static const bool loadFromFile(ifstream&);
		static const bool exists(const Tag& tag) { if (leadershipDistributor.count(tag) && leadershipDistributor.at(tag)) return true; return false; }
		static LeadershipDistributor& get(const Tag& tag) { return *leadershipDistributor.at(tag); }

	private:

		LeadershipDistributor(const LeadershipDistributor&) = default;
		LeadershipDistributor(LeadershipDistributor&&) = default;
		LeadershipDistributor();

		void distributeLeadership();

		float                                                                wastedLeadership = 0.0f;
		pair<float, float>                                                   leadership = make_pair(BASE_LEADERSHIP, 1.0f);
		mutable map<LeadershipDistributionCategory, std::pair<double, bool>> leadershipDistribution;

		const Tag tag;

		static unordered_map<Tag, unique_ptr<LeadershipDistributor>> leadershipDistributor;
		static const float BASE_LEADERSHIP;
	};
}
#endif
