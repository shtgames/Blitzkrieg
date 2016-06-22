#ifndef LEADERSHIP_DISTRIBUTOR_BACKEND
#define LEADERSHIP_DISTRIBUTOR_BACKEND

#include "Tag.h"
#include "FileProcessor.h"

#include <fstream>
#include <unordered_map>
#include <map>
#include <memory>
#include <atomic>
#include <mutex>

using namespace std;

namespace bEnd
{
	class Province;
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

		void loadFromSave(const FileProcessor::Statement& source);

		const float getLeadershipDistributionAmount(const LeadershipDistributionCategory category)const { lock_guard<mutex> guard(leadershipDistributionLock); return leadershipDistribution[category].first * leadership.first * leadership.second; }

		void setLeadershipDistributionValue(const LeadershipDistributionCategory category, const double factor);
		void setLeadershipDistributionValueLock(const LeadershipDistributionCategory category, const bool lock = false);

		const float getLeadershipAmount()const { return leadership.first * leadership.second; }
		void changeLeadershipAmount(const float amount) { leadership.first + amount >= 0.0f ? leadership.first + amount : leadership.first = 0.0f; }

		void update();
		void reset();

		static const bool exists(const Tag& tag) { if (leadershipDistributor.count(tag) && leadershipDistributor.at(tag)) return true; return false; }
		static void emplace(const Tag& tag) { leadershipDistributor[tag].reset(new LeadershipDistributor(tag)); }
		static LeadershipDistributor& get(const Tag& tag) { if (!leadershipDistributor.count(tag)) emplace(tag); return *leadershipDistributor.at(tag); }

	private:

		LeadershipDistributor(const LeadershipDistributor&) = default;
		LeadershipDistributor(LeadershipDistributor&&) = default;
		LeadershipDistributor() = delete;
		LeadershipDistributor(const Tag& tag);

		void distributeLeadership();

		atomic<float>                                                        wastedLeadership = 0.0f;
		pair<atomic<float>, atomic<float>>                                   leadership = make_pair(BASE_LEADERSHIP, 1.0f);
		mutable map<LeadershipDistributionCategory, std::pair<double, bool>> leadershipDistribution;

		const Tag tag;

		mutable mutex leadershipDistributionLock;

		static unordered_map<Tag, unique_ptr<LeadershipDistributor>> leadershipDistributor;
		static const float BASE_LEADERSHIP;
	};
}
#endif
