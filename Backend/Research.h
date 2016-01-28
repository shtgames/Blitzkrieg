#ifndef RESEARCH_BACKEND
#define RESEARCH_BACKEND

#include "Tag.h"
#include "Date.h"

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <fstream>

using namespace std;

namespace bEnd
{
	class Research final
	{
	public:

		~Research() = default;

		void increaseResearchItemPriority(const unsigned short);
		void decreaseResearchItemPriority(const unsigned short);
		void setResearchItemAtMaxPriority(const unsigned short);
		void setResearchItemAtMinPriority(const unsigned short);
		void removeResearchItem(const unsigned short);
		void addResearchItem(const string&);

		void addExperienceRewards(const vector<pair<std::string, float>>&);
		const float setLeadership(const float leadership);
		void update();

		const map<string, float>& getExperience()const { return experience; }

		static const bool loadFromFile(ifstream& file);
		static const bool exists(const Tag& tag) { if (research.count(tag) && research.at(tag)) return true; return false; }
		static Research& getResearch(const Tag& tag) { return *research.at(tag); };

	private:

		class ResearchItem final
		{
		public:
			ResearchItem(const string& tech, const unsigned short days) : tech(tech), researchDays(days) {}
			ResearchItem(const ResearchItem&) = default;
			ResearchItem(ResearchItem&&) = default;
			ResearchItem() = default;
			~ResearchItem() = default;

			ResearchItem& operator=(const ResearchItem&) = default;
			ResearchItem& operator=(ResearchItem&&) = default;

			const string& getTech()const { return tech; }
			const double getCompletionPercentage()const { return completionPercentage; }
			const Date getComlpetionDate()const;

			ResearchItem& setLeadership(const float dedicatedLS) { if (dedicatedLS >= 0.0f) dedicatedLeadership = dedicatedLS <= 1.0f ? dedicatedLS : 1.0f; return *this; }
			ResearchItem& setResearchDays(const unsigned short days) { researchDays = days; return *this; }

			const bool research();
		private:
			const string tech;

			unsigned short researchDays = 0;
			double completionPercentage = 0.0f;
			float dedicatedLeadership = 0.0f;
		};

		Research(const Research&);
		Research(Research&&) = default;
		Research() = default;

		map<string, float>               experience;
		map<string, unsigned char>       techLevels;
		vector<unique_ptr<ResearchItem>> researchQueue;
		float                            totalDedicatedLeadership = 0.0f;

		static unordered_map<Tag, unique_ptr<Research>> research;

		static const float EXPERIENCE_CAP;
	};
}

#endif