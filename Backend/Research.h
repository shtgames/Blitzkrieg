#ifndef RESEARCH_BACKEND
#define RESEARCH_BACKEND

#include "Tag.h"
#include "Date.h"
#include "FileProcessor.h"

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <fstream>
#include <atomic>
#include <mutex>

using namespace std;

namespace bEnd
{
	class Tech;
	class Research final
	{
	public:
		~Research() = default;

		const bool loadFromSave(const FileProcessor::Statement& source);

		void increaseResearchItemPriority(const unsigned short index);
		void decreaseResearchItemPriority(const unsigned short index);
		void setResearchItemAtMaxPriority(const unsigned short index);
		void setResearchItemAtMinPriority(const unsigned short index);
		void removeResearchItem(const unsigned short index);
		void addResearchItem(const string& key);

		void addExperienceRewards(const std::string& target, const float amount);
		const float setLeadership(const float leadership);
		void update();

		const float getTechLevel(const std::string& tech)const { std::lock_guard<std::mutex> guard(techLevelsLock); return techLevels.count(tech) ? float(techLevels.at(tech)) : techLevels[tech] = 0.0f; }
		const float getExperience(const std::string& name)const { std::lock_guard<std::mutex> guard(experienceLock); return experience.count(name) ? float(experience.at(name)) : experience[name] = 0.0f; }

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

			atomic<unsigned short> researchDays = 0;
			atomic<float>          completionPercentage = 0.0f;
			atomic<float>          dedicatedLeadership = 0.0f;
		};

		Research(const Tag& tag) : tag(tag) {}
		Research(Research&&) = default;
		Research() = delete;

		mutable map<string, atomic<float>> techLevels;
		mutable map<string, atomic<float>> experience;
		map<string, atomic<bool>>          experienceHasBeenAdded;
		vector<unique_ptr<ResearchItem>>   researchQueue;
		atomic<float>                      totalDedicatedLeadership = 0.0f;
		const Tag                          tag;
		mutable mutex                      researchQueueLock, techLevelsLock, experienceLock;

		static unordered_map<Tag, unique_ptr<Research>> research;

		static const float EXPERIENCE_CAP, EXPERIENCE_DECAY_PERCENTAGE;
	};
}

#endif