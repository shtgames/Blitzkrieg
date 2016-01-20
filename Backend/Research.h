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

		void increaseResearchItemPriority(const unsigned short);
		void decreaseResearchItemPriority(const unsigned short);
		void setResearchItemAtMaxPriority(const unsigned short);
		void setResearchItemAtMinPriority(const unsigned short);
		void removeResearchItem(const unsigned short);
		void addResearchItem(const string&);

		const float setLeadership(const float Leadership);
		void update();

		static const bool loadFromFile(ifstream&);

		static Research& getResearch(const Tag& tag) { return research[tag]; };

	private:
		class ResearchItem final
		{
		public:
			ResearchItem(const string& tech) : tech(tech) {}
			ResearchItem(const ResearchItem&) = default;
			ResearchItem(ResearchItem&&) = default;
			ResearchItem() = default;
			~ResearchItem() = default;

			ResearchItem& operator=(const ResearchItem&) = default;
			ResearchItem& operator=(ResearchItem&&) = default;

			const string& getTech()const { return tech; }
			const double getCompletionPercentage()const { return completionPercentage; }
			const Date getComlpetionDate()const;

			ResearchItem& setLeadership(const float dedicatedLS) { if (dedicatedLS >= 0.0f) dedicatedLeadership = dedicatedLS; return *this; }
			ResearchItem& setResearchDays(const unsigned short days) { researchDays = days; return *this; }

			const bool research();
		private:
			const string tech;

			unsigned short researchDays = 0.0f;
			double completionPercentage = 0.0f;
			float dedicatedLeadership = 0.0f;
		};

		Research(const Research&) = default;
		Research(Research&&) = default;
		Research() = default;
		~Research() = default;

		Research& operator=(const Research&) = default;
		Research& operator=(Research&&) = default;

		map<string, float>               experience;
		map<string, unsigned char>       techLevels;
		vector<unique_ptr<ResearchItem>> researchQueue;

		static unordered_map<Tag, Research> research;
	};
}

#endif