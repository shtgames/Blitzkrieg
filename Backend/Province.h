#ifndef Province_BACKEND
#define Province_BACKEND

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

namespace fEnd
{
	class Map;
	class MapLoader;
}

namespace bEnd
{
	class Unit;
	class Province final
	{
		friend class fEnd::Map;
		friend class fEnd::MapLoader;
	public:
		typedef pair<float, unsigned char>(BuildingLevels);

		Province(const Province&) = default;
		Province(Province&&) = default;
		Province();
		~Province() = default;

		void build(const Unit& building);
		void repairAll();

		const Tag& getOwner()const { return owner; }
		const Tag& getController()const { return controller; }
		const float getLeadership()const;
		const float getManpowerGeneration()const;
		const float getResourceGeneration(const Resource resource)const;
		const float getIC()const;
		const BuildingLevels getBuildingLevels(const std::string& key);
		const bool isSea()const;
		const bool isCoastal()const;

		bool hasCore(const Tag&)const;
		void addCore(const Tag&);

		void enqueueBuilding(const std::string& key, const unsigned char amount = 1);
		void dequeueBuilding(const std::string& key, unsigned char amount = 1);
		const unsigned char getQueuedCount(const std::string& key);

		const std::unordered_map<unsigned short, unsigned short>& getNeighbours()const;
		const std::set<unsigned short> getPath(const unsigned short target)const;

		static void loadFromSave(const FileProcessor::Statement& source);
		static const bool exists(const unsigned short ID) { return provinces.count(ID); }
		static Province& get(const unsigned short ProvinceID) { return provinces[ProvinceID]; }
		
	private:		
		void changeOwner(const Tag&);
		void changeController(const Tag&);
		void generateResources();
		void stopGeneratingResources();
		void reset();

		void repair(const Unit& building, float levels);
		
		std::atomic<unsigned char>                                victoryPoints = {0};
		std::atomic<bool>                                         sea = {false}, coastal = {false},
				initialized = {false}, capital = {false}, generatingResources = {false};
		mutable map<string, BuildingLevels>                       buildings;
		mutable map<Resource, pair<atomic<float>, atomic<float>>> resourceGeneration;
		pair<atomic<float>, atomic<float>>                        leadership, IC, manpowerGeneration;
		Tag                                                       owner, controller;
		set<Tag>                                                  cores;
		std::atomic<unsigned short>                               provID = {0};
		std::unordered_map<std::string, unsigned char>            queuedBuildingCount;
		std::unordered_map<unsigned short, unsigned short>        neighbours; // Key is province index, value is distance to province.

		mutable std::mutex coresLock, resourceLock, queueLock;

		static map<unsigned short, Province> provinces;
		static const float ANNEXED_NON_CORE_PENALTY;
	};
}
#endif
