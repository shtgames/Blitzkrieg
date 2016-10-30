#ifndef POLITICS_BACKEND
#define POLITICS_BACKEND

#include "Tag.h"
#include "OccupationPolicy.h"

#include <unordered_map>
#include <memory>
#include <fstream>
#include <atomic>
#include <mutex>

using namespace std;

namespace bEnd
{
	class Politics final
	{
	public:
		~Politics() = default;

		const float getNationalUnity()const { return nationalUnity; };
		const float getDissent()const { return dissent; };
		const float getDissentChange()const { return dissentChange; };
		const Policy getOccupationPolicy(const Tag& tag)const { lock_guard<mutex> guard(occupationPoliciesLock); return occupationPolicies[tag]; };

		void update();

		static const bool loadFromFile(ifstream&);
		static const bool exists(const Tag& tag) { if (politics.count(tag) && politics.at(tag)) return true; return false; }
		static void emplace(const Tag& tag) { politics[tag].reset(new Politics()); }
		static Politics& get(const Tag& tag) { if (!politics.count(tag)) emplace(tag); return *politics.at(tag); };

	private:
		Politics(const Politics&) = default;
		Politics(Politics&&) = default;
		Politics() = default;

		atomic<float> nationalUnity = {50.0f}, dissent = {0.0f}, dissentChange = {0.0f};
		mutable unordered_map<Tag, Policy> occupationPolicies;

		mutable mutex occupationPoliciesLock;

		static unordered_map<Tag, unique_ptr<Politics>> politics;
	};
}

#endif
