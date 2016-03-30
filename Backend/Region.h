#ifndef REGION_BACKEND
#define REGION_BACKEND

#include "Resources.h"
#include "Tag.h"
#include "FileProcessor.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <set>
#include <fstream>
#include <atomic>

using namespace std;

namespace bEnd
{
	class Unit;
	class Region final
	{
	public:
		~Region() = default;

		void build(const Unit& building);
		void repairAll();

		const Tag& getOwner()const { return owner; }
		const Tag& getController()const { return controller; }
		const float getLeadership()const;
		const float getManpowerGeneration()const;
		const float getResourceGeneration(const Resource resource)const;
		const float getIC()const;

		bool hasCore(const Tag&)const;
		void addCore(const Tag&);

		static const bool loadFromSave(const FileProcessor::Statement& source);
		static const bool exists(const unsigned short ID) { return regions.count(ID); }
		static Region& get(const unsigned short regionID) { return regions.at(regionID); }

	private:
		typedef pair<float, unsigned char>(BuildingLevels);

		Region(const Region&) = default;
		Region(Region&&) = default;
		Region();

		void changeOwner(const Tag&);
		void changeController(const Tag&);
		void generateResources();
		void stopGeneratingResources();

		void repair(const Unit& building, float levels);
		
		std::atomic<unsigned char>                                victoryPoints = 0;
		std::atomic<bool>                                         sea = false, initialized = false, capital = false, generatingResources = false;
		mutable map<string, BuildingLevels>                       buildings;
		mutable map<Resource, pair<atomic<float>, atomic<float>>> resourceGeneration;
		pair<atomic<float>, atomic<float>>                        leadership = std::make_pair(0.0f, 1.0f), IC = std::make_pair(0.0f, 1.0f), manpowerGeneration = std::make_pair(0.0f, 1.0f);
		Tag                                                       owner, controller;
		set<Tag>                                                  cores;
		std::atomic<unsigned short>                               provID = 0;

		mutable std::mutex coresLock, resourceLock;

		static map<unsigned short, Region> regions;
		static const float ANNEXED_NON_CORE_PENALTY;
	};
}
#endif