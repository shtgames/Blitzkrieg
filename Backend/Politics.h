#ifndef POLITICS_BACKEND
#define POLITICS_BACKEND

#include "Tag.h"
#include "OccupationPolicy.h"

#include <unordered_map>
#include <fstream>

using namespace std;

namespace bEnd
{
	class Politics final
	{
	public:

		const float& getNationalUnity()const { return nationalUnity; };
		const float& getDissent()const { return dissent; };
		const float& getDissentChange()const { return dissentChange; };
		const bEnd::OccupationPolicy& getOccupationPolicy(const Tag& tag)const { return occupationPolicies[tag]; };

		void update();

		static const bool loadFronFile(ifstream&);

		static Politics& getPolitics(const Tag& tag) { return politics[tag]; };

	private:

		Politics(const Politics&) = default;
		Politics(Politics&&) = default;
		~Politics() = default;
		Politics();

		Politics& operator=(const Politics&) = default;
		Politics& operator=(Politics&&) = default;

		float nationalUnity = 50.0f, dissent = 0.0f, dissentChange = 0.0f;
		mutable unordered_map<Tag, bEnd::OccupationPolicy> occupationPolicies;

		static unordered_map<Tag, Politics> politics;
	};
}

#endif