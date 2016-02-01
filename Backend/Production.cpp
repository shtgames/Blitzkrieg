#include "Region.h"

#include "Production.h"

#include "Unit.h"
#include "Research.h"
#include "Region.h"
#include "ResourceDistributor.h"
#include "TimeSystem.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<Production>> Production::production;

	void Production::increaseProductionItemPriority(const unsigned short index)
	{
		if (index < productionLine.size() && index != 0)
		{
			productionLine.at(index).swap(productionLine.at(index - 1));
			setIC(totalDedicatedIC);
		}
	}

	void Production::decreaseProductionItemPriority(const unsigned short index)
	{
		if (index < productionLine.size() - 1)
		{
			productionLine.at(index).swap(productionLine.at(index + 1));
			setIC(totalDedicatedIC);
		}
	}

	void Production::setProductionItemAtMaxPriority(const unsigned short index) 
	{
		if (index < productionLine.size() && index != 0)
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
		if (index < productionLine.size()) 
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
		if (Unit::exists(element) && ResourceDistributor::get(tag).getManpowerAmount() <= Unit::get(element).getRequiredManpower())
		{
			productionLine.emplace_back(new ProductionItem(Unit::get(element),
				Unit::get(element).getProductionDays(Research::get(tag).getExperience()), 
				targetRegion));
			ResourceDistributor::get(tag).changeManpowerAmount((-1) * Unit::get(element).getRequiredManpower());
			setIC(totalDedicatedIC);
		}
	}

	const float Production::setIC(float IC)
	{
		if (IC < 0.0f) IC = 0.0f;

		totalDedicatedIC = IC;
		for (auto it = productionLine.begin(), end = productionLine.end(); it != end; ++it)
			if (*it)
			{
				const float requiredIC = (*it)->getUnit().getRequiredIC(Research::get(tag).getExperience());
				(*it)->setIC(IC / requiredIC);
				IC > requiredIC ? IC -= requiredIC : IC = 0.0f;
			}
			else it = productionLine.erase(it);
		return IC;
	}

	void Production::update()
	{
		for (auto it = productionLine.begin(), end = productionLine.end(); it != end; ++it)
			if (*it)
			{
				(*it)->updateProductionDays(tag);
				if ((*it)->produce())
				{
					if (deploy(**it, (*it)->getTarget()));
					else if ((*it)->getUnit().isDelayDeployable()) awaitingDeployment.push_back(std::move(*it));
					Research::get(tag).addExperienceRewards((*it)->getUnit().getExperienceRewards());
					it = productionLine.erase(it);
				}
			}
			else it = productionLine.erase(it);
	}
	
	const bool Production::loadFromFile(ifstream& file)
	{
		return false;
	}

	const bool Production::deploy(const ProductionItem& item, const unsigned short targetRegion)
	{
		if (targetRegion != unsigned short(-1) && Region::exists(targetRegion) && Region::get(targetRegion).getController() == tag)
		{
			switch (item.getUnit().getType())
			{
			case Unit::Building:
				Region::get(item.getTarget()).build(item.getUnit());
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

	Production::ProductionItem::ProductionItem(const Unit& tech, const unsigned short days, const unsigned short region)
		: unit(tech), productionDays(days), targetRegion(Region::exists(region) ? region : -1) {}

	const Date Production::ProductionItem::getComlpetionDate() const
	{
		if (dedicatedICPercentage > 0.0f)
			return (TimeSystem::getCurrentDate() + Date((((100.0f - completionPercentage) / 100.0f) * (1.0f / dedicatedICPercentage) * productionDays) * 24));
		else return Date::NEVER;
	}

	Production::ProductionItem& Production::ProductionItem::updateProductionDays(const Tag& tag)
	{
		productionDays = unit.getProductionDays(Research::get(tag).getExperience());
		return *this;
	}

	const bool Production::ProductionItem::produce()
	{
		if (dedicatedICPercentage > 0.0f) completionPercentage += (100.0f / (productionDays * (1.0f / dedicatedICPercentage)));
		return completionPercentage >= 100.0f;
	}
};