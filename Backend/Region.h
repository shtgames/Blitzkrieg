#ifndef REGION_BACKEND
#define REGION_BACKEND


#include "Resources.h"
#include "Tag.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <set>
#include <fstream>

using namespace std;

namespace bEnd
{
	class Unit;
	class Region final
	{
	public:
		typedef float(BuildingLevel);
		typedef pair<BuildingLevel, BuildingLevel>(BuildingLevels);
		
		~Region() = default;

		const Tag& getOwner()const { return owner; }
		const Tag& getController()const { return controller; }
		const float getLeadership()const;
		const float getManpowerGeneration()const;
		const float getResourceGeneration(const Resource resource)const;
		const float getIC()const;

		bool hasCore(const Tag&)const;
		void addCore(const Tag&);
		void build(const Unit& building);

		static const bool exists(const unsigned short ID) { return regions.count(ID); }
		static const bool loadFromFile(ifstream& file);

		static Region& get(const unsigned short regionID) { return regions.at(regionID); }

	private:

		Region(const Region&) = default;
		Region(Region&&) = default;
		Region();

		void changeOwner(const Tag&);
		void changeController(const Tag&);
		void generateResources();
		void stopGeneratingResources();
		
		bool                                      sea = false, initialized = false, capital = false;
		mutable map<string, BuildingLevels>       buildings;
		mutable map<Resource, pair<float, float>> resourceGeneration;
		pair<float, float>                        leadership = std::make_pair(0.0f, 1.0f), IC = std::make_pair(0.0f, 1.0f), manpowerGeneration = std::make_pair(0.0f, 1.0f);
		Tag                                       owner, controller;
		set<Tag>                                  cores;
		unsigned short                            provID = 0;

		static map<unsigned short, Region> regions;
		static const float ANNEXED_NON_CORE_PENALTY;
	};
}
#endif