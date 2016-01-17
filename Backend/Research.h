#ifndef RESEARCH_BACKEND
#define RESEARCH_BACKEND

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace bEnd
{
	class Date;
	class Research final
	{
	public:
		Research(const Research&) = default;
		Research(Research&&) = default;
		Research() = default;
		~Research() = default;

		Research& operator=(const Research&) = default;
		Research& operator=(Research&&) = default;

		void increaseResearchItemPriority(const unsigned short);
		void decreaseResearchItemPriority(const unsigned short);
		void setResearchItemAtMaxPriority(const unsigned short);
		void setResearchItemAtMinPriority(const unsigned short);
		void removeResearchItem(const unsigned short);
		void addResearchItem(const std::string&);

		void update();

		const float setLeadership(const float);

	private:
		class ResearchItem final
		{
		public:
			ResearchItem(const std::string& tech) : tech(tech) {}
			ResearchItem(const ResearchItem&) = default;
			ResearchItem(ResearchItem&&) = default;
			ResearchItem() = default;
			~ResearchItem() = default;

			ResearchItem& operator=(const ResearchItem&) = default;
			ResearchItem& operator=(ResearchItem&&) = default;

			const std::string& getTech()const { return tech; }
			const double getCompletionPercentage()const { return completionPercentage; }
			const Date getComlpetionDate()const;

			ResearchItem& setLeadership(const float dedicatedLS) { if (dedicatedLS >= 0.0f) dedicatedLeadership = dedicatedLS; return *this; }
			ResearchItem& setResearchDays(const unsigned short days) { researchDays = days; return *this; }

			const bool research();
		private:
			const std::string tech;

			unsigned short researchDays = 0.0f;
			double completionPercentage = 0.0f;
			float dedicatedLeadership = 0.0f;
		};
		
		std::map<std::string, float>               experience;
		std::map<std::string, unsigned char>       techLevels;
		std::vector<std::unique_ptr<ResearchItem>> researchQueue;

		friend class Region;
		friend class Nation;
	};
}

#endif