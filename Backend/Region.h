#ifndef REGION_BACKEND
#define REGION_BACKEND


#include "ProductionItem.h"
#include "Tag.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <set>

namespace bEnd
{
	class Region final
	{
	public:
		enum Structure
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
		typedef char(QueuedBuildingAmount);
		typedef std::pair<BuildingLevel, BuildingLevel>(Building);

		class Construction : public bEnd::ProductionItem
		{
		public:
			Construction(const Structure, const unsigned short);
			void onCompletion();
		private:
			const unsigned short targetRegion;
			const Structure buildingType;
		};

		const Tag& getOwner()const { return owner; }
		const Tag& getController()const { return controller; }
		const float getLeadershipGeneration()const { return leadershipGeneration; }
		bool hasCore(const Tag&)const;

		void generateResources(std::map<Tag, Nation>&);
		void addCore(const Tag&);

		static void generateResourcesGlobal(std::map<Tag, Nation>&);

	private:
		void build(const Structure);
		void changeOwner(const Tag&);
		void changeController(const Tag&);

		static std::unordered_map<unsigned short, Region> regions;

		bool                                                           sea = false, initialized = false, capital = false;
		std::map<Structure, std::pair<Building, QueuedBuildingAmount>> buildings;
		std::map<bEnd::Resource, float>                                resourceGeneration;
		float                                                          leadershipGeneration = 0.0f, manpowerGeneration = 0.0f;
		Tag                                                            owner, controller;
		std::set<Tag>                                                  cores;
		unsigned short                                                 provID = 0;
	};
}
#endif