#include "Nation.h"

#include "LeadershipDistributor.h"
#include "Research.h"
#include "ResourceDistributor.h"
#include "Production.h"

namespace bEnd
{
	bEnd::Tag bEnd::Nation::player;

	void Nation::loadFromSave(const FileProcessor::Statement& source)
	{
		for (auto it = source.rStatements.begin(), end = source.rStatements.end(); it != end; ++it)
			if (it->lValue == "technology")
				Research::get(source.lValue).loadTechLevels(*it);
			else if (it->lValue == "research")
				Research::get(source.lValue).loadResearchItem(*it);
			else if (it->lValue == "leadership")
				LeadershipDistributor::get(source.lValue).loadFromSave(*it);
			else if (it->lValue == "military_construction" || it->lValue == "building_construction")
				Production::get(source.lValue).loadProductionItem(*it);
	}

	void Nation::reset()
	{
		ResourceDistributor::reset();
	}
}
