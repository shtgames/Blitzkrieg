#include "Nation.h"

#include "LeadershipDistributor.h"
#include "Research.h"
#include "ResourceDistributor.h"
#include "Production.h"

void bEnd::Nation::loadFromSave(const FileProcessor::Statement& source)
{
	for (auto it = source.rStatements.begin(), end = source.rStatements.end(); it != end; ++it)
		if (it->lValue == "technology")
			Research::get(source.lValue).loadTechLevels(*it);
		else if (it->lValue == "research")
			Research::get(source.lValue).loadResearchItem(*it);
		else if (it->lValue == "distribution")
			ResourceDistributor::get(source.lValue).loadFromSave(*it);
		else if (it->lValue == "leadership")
			LeadershipDistributor::get(source.lValue).loadFromSave(*it);
		else if (it->lValue == "military_construction" || it->lValue == "building_construction")
			Production::get(source.lValue).loadProductionItem(*it);
		else if (it->lValue == "nocategory")
			while (it != end && it->lValue != "research")
			{
				Research::get(source.lValue).loadExperienceLevels(*it);
				++it;
			}
}
