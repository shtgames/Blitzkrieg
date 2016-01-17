#ifndef POLITICS_BACKEND
#define POLITICS_BACKEND

#include "OccupationPolicy.h"
#include "Tag.h"

#include <unordered_map>

namespace bEnd
{
	class Politics final
	{
	public:
		Politics(const Politics&) = default;
		Politics(Politics&&) = default;
		~Politics() = default;
		Politics();

		Politics& operator=(const Politics&) = default;
		Politics& operator=(Politics&&) = default;

		const float& getNationalUnity()const { return NationalUnity; };
		const float& getDissent()const { return dissent; };
		const float& getDissentChange()const { return dissentChange; };
		const OccupationPolicy& getOccupationPolicy(const Tag& tag)const { if (occupationPolicies.count(tag)) return occupationPolicies.at(tag); else return OccupationPolicy(); };

		void update();

	private:

		float NationalUnity, dissent, dissentChange;

		std::unordered_map<Tag, OccupationPolicy> occupationPolicies;
	};
}

#endif