#ifndef REGION_BACKEND
#define REGION_BACKEND


#include "ProductionItem.h"
#include "Resources.h"
#include "Tag.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <set>

using namespace std;

namespace bEnd
{
	class Region final
	{
	public:
		enum BuildingType
		{
			Airbase,
			NavalBase,
			CoastalFort,
			LandFort,
			AA,
			Radar,
			NuclearReactor,
			RocketTestSite,
			Industry,
			Infrastructure
		};

		typedef char(BuildingLevel);
		typedef unsigned char(QueuedBuildingAmount);
		typedef pair<BuildingLevel, BuildingLevel>(BuildingLevels);

		class Construction : public ProductionItem
		{
		public:
			Construction(const BuildingType, const unsigned short);
			~Construction();

			void onCompletion();
		private:
			const unsigned short targetRegion;
			const BuildingType buildingType;
		};

		const Tag& getOwner()const { return owner; }
		const Tag& getController()const { return controller; }
		const float getLeadershipGeneration()const;
		const float getManpowerGeneration()const;
		const float getResourceGeneration(const Resource resource)const;
		const float getIC()const;

		bool hasCore(const Tag&)const;
		void addCore(const Tag&);

	private:
		void changeOwner(const Tag&);
		void changeController(const Tag&);

		static unordered_map<unsigned short, Region> regions;

		bool                                                                  sea = false, initialized = false, capital = false;
		mutable map<BuildingType, pair<BuildingLevels, QueuedBuildingAmount>> buildings;
		mutable map<Resource, float>                                          resourceGeneration;
		float                                                                 leadershipGeneration = 0.0f, manpowerGeneration = 0.0f;
		Tag                                                                   owner, controller;
		set<Tag>                                                              cores;
		unsigned short                                                        provID = 0;

		static const float ANNEXED_NON_CORE_PENALTY, IC_POINTS_PER_LEVEL;
	};
}
#endif