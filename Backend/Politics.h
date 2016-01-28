#ifndef POLITICS_BACKEND
#define POLITICS_BACKEND

#include "Tag.h"
#include "OccupationPolicy.h"

#include <unordered_map>
#include <memory>
#include <fstream>

using namespace std;

namespace bEnd
{
	class Politics final
	{
	public:

		~Politics() = default;

		const float& getNationalUnity()const { return nationalUnity; };
		const float& getDissent()const { return dissent; };
		const float& getDissentChange()const { return dissentChange; };
		const OccupationPolicy& getOccupationPolicy(const Tag& tag)const { return occupationPolicies[tag]; };

		void update();

		static const bool loadFronFile(ifstream&);
		static const bool exists(const Tag& tag) { if (politics.count(tag) && politics.at(tag)) return true; return false; }
		static Politics& getPolitics(const Tag& tag) { return *politics.at(tag); };

	private:

		Politics(const Politics&) = default;
		Politics(Politics&&) = default;
		Politics();

		float nationalUnity = 50.0f, dissent = 0.0f, dissentChange = 0.0f;
		mutable unordered_map<Tag, OccupationPolicy> occupationPolicies;

		static unordered_map<Tag, unique_ptr<Politics>> politics;
	};
}

#endif