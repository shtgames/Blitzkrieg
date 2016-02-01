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
	class Tech;
	class Research final
	{
	public:

		~Research() = default;

		void increaseResearchItemPriority(const unsigned short index);
		void decreaseResearchItemPriority(const unsigned short index);
		void setResearchItemAtMaxPriority(const unsigned short index);
		void setResearchItemAtMinPriority(const unsigned short index);
		void removeResearchItem(const unsigned short index);
		void addResearchItem(const string& key);

		void addExperienceRewards(const vector<pair<std::string, float>>& rewards);
		const float setLeadership(const float leadership);
		void update();

		const float getTechLevel(const std::string tech) { return techLevels.at(tech); }
		const map<string, float>& getExperience()const { return experience; }

		static const bool loadFromFile(ifstream& file);
		static const bool exists(const Tag& tag) { if (research.count(tag) && research.at(tag)) return true; return false; }
		static Research& get(const Tag& tag) { return *research.at(tag); };

	private:

		class ResearchItem final
		{
		public:
			ResearchItem(const Tech& tech, const unsigned short days, const float percentage = 0.0f) : tech(tech), researchDays(days) {
				if (percentage >= 0.0f && percentage < 100.0f) completionPercentage = percentage; 
				else completionPercentage = 0.0f;
			}
			ResearchItem(const ResearchItem&) = default;
			ResearchItem(ResearchItem&&) = default;
			ResearchItem() = delete;
			~ResearchItem() = default;
			
			const Tech& getTech()const { return tech; }
			const double getCompletionPercentage()const { return completionPercentage; }
			const Date getComlpetionDate()const;

			ResearchItem& setLeadership(const float dedicatedLS) { if (dedicatedLS >= 0.0f) dedicatedLeadership = dedicatedLS <= 1.0f ? dedicatedLS : 1.0f; return *this; }
			ResearchItem& updateResearchDays(const Tag& tag);

			const bool research();
		private:
			const Tech& tech;

			unsigned short researchDays = 0;
			float completionPercentage = 0.0f;
			float dedicatedLeadership = 0.0f;
		};

		Research(const Tag& tag) : tag(tag) {}
		Research(const Research&);
		Research(Research&&) = default;
		Research() = delete;

		map<string, float>               experience;
		map<string, bool>                experienceHasBeenAdded;
		map<string, float>               techLevels;
		vector<unique_ptr<ResearchItem>> researchQueue;
		float                            totalDedicatedLeadership = 0.0f;
		const Tag                        tag;

		static unordered_map<Tag, unique_ptr<Research>> research;

		static const float EXPERIENCE_CAP, EXPERIENCE_DECAY_PERCENTAGE;
	};
}

#endif