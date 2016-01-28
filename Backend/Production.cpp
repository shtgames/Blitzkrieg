#include "Region.h"

#include "Production.h"

#include "Unit.h"
#include "Research.h"
#include "Region.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<Production>> Production::production;

	void Production::increaseProductionItemPriority(const unsigned short index)
	{
		if (index < productionLine.size() - 1 && index != 0)
		{
			productionLine.at(index).swap(productionLine.at(index - 1));
			setIC(totalDedicatedIC);
		}
	}

	void Production::decreaseProductionItemPriority(const unsigned short index)
	{
		if (index < productionLine.size() - 1 && index != productionLine.size() - 2)
		{
			productionLine.at(index).swap(productionLine.at(index + 1));
			setIC(totalDedicatedIC);
		}
	}

	void Production::setProductionItemAtMaxPriority(const unsigned short index) 
	{
		if (index < productionLine.size() - 1)
		{
			productionLine.at(index).swap(productionLine.front());
			setIC(totalDedicatedIC);
		}
	}

	void Production::setProductionItemAtMinPriority(const unsigned short index)
	{
		if (index < productionLine.size() - 1)
		{
			productionLine.at(index).swap(productionLine.back());  
			setIC(totalDedicatedIC);
		}
	}

	void Production::removeProductionItem(const unsigned short index)
	{
		if (index < productionLine.size() - 1) 
		{
			productionLine.erase(productionLine.begin() + index);
			setIC(totalDedicatedIC); 
		}
	}

	void Production::deployUnit(const unsigned short index, const unsigned short targetRegion)
	{
		if (index < awaitingDeployment.size() - 1 && awaitingDeployment.at(index) && deploy(*awaitingDeployment.at(index), targetRegion))
			awaitingDeployment.erase(awaitingDeployment.begin() + index);
	}

	void Production::addProductionItem(const string& element, const unsigned short targetRegion)
	{
		if (Unit::unitExists(element))
		{
			productionLine.emplace_back(new ProductionItem(element, Unit::getUnit(element).getProductionDays(Research::getResearch(tag).getExperience()), Region::regionExists(targetRegion) ? targetRegion : -1));
			setIC(totalDedicatedIC);
		}
	}

	const float Production::setIC(float IC)
	{
		if (IC < 0.0f) IC = 0.0f;

		totalDedicatedIC = IC;
		for (auto it = productionLine.begin(), end = productionLine.end(); it != end; ++it)
			if (*it && Unit::exists((*it)->getUnit()))
			{
				const Unit& unit = Unit::getUnit((*it)->getUnit());
				const float requiredIC = unit.getRequiredIC(Research::getResearch(tag).getExperience());
				(*it)->setIC(IC / requiredIC);
				(*it)->setProductionDays(unit.getProductionDays(Research::getResearch(tag).getExperience()));
				if (IC > requiredIC) IC -= requiredIC;
				else IC = 0.0f;
			}
			else it = productionLine.erase(it);
		return IC;
	}

	void Production::update()
	{
		for (auto it = productionLine.begin(), end = productionLine.end(); it != end; ++it)
			if (*it && (*it)->produce())
			{
				if (Unit::exists((*it)->getUnit()))
				{
					if (deploy(**it, (*it)->getTargetRegion()));
					else if (Unit::getUnit((*it)->getUnit()).isDelayDeployable()) awaitingDeployment.push_back(std::move(*it));
					Research::getResearch(tag).addExperienceRewards(Unit::getUnit((*it)->getUnit()).getExperienceRewards());
				}
				it = productionLine.erase(it);
			}
	}
	
	const bool Production::loadFromFile(ifstream& file)
	{
		return false;
	}

	const bool Production::deploy(const ProductionItem& item, const unsigned short targetRegion)
	{
		if (targetRegion != unsigned short(-1) && Region::regionExists(targetRegion) && Region::getRegion(targetRegion).getController() == tag)
		{
			switch (Unit::getUnit(item.getUnit()).getType())
			{
			case Unit::Building:
				Region::getRegion(item.getTargetRegion()).build(item.getUnit());
			case Unit::Land:
				///
			case Unit::Air:
				///
			case Unit::Naval:
				///
				break;
			};
			return true;
		}
		return false;
	}

	const Date Production::ProductionItem::getComlpetionDate() const
	{
		return Date();
	}

	const bool Production::ProductionItem::produce()
	{
		return false;
	}
};