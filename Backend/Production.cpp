#include "Region.h"

#include "Production.h"

#include "Unit.h"
#include "Research.h"
#include "Region.h"
#include "ResourceDistributor.h"
#include "TimeSystem.h"

namespace bEnd
{
	std::unordered_map<Tag, std::unique_ptr<Production>> Production::production;

	void Production::loadProductionItem(const FileProcessor::Statement& file)
	{
		
	}

	void Production::increaseProductionItemPriority(const unsigned short index)
	{
		productionLineLock.lock();
		if (index < productionLine.size() && index != 0)
		{
			productionLine.at(index).swap(productionLine.at(index - 1));
			setIC(totalDedicatedIC);
		}
		productionLineLock.unlock();
	}

	void Production::decreaseProductionItemPriority(const unsigned short index)
	{
		productionLineLock.lock();
		if (index < productionLine.size() - 1)
		{
			productionLine.at(index).swap(productionLine.at(index + 1));
			setIC(totalDedicatedIC);
		}
		productionLineLock.unlock();
	}

	void Production::setProductionItemAtMaxPriority(const unsigned short index) 
	{
		productionLineLock.lock();
		if (index < productionLine.size() && index != 0)
		{
			productionLine.at(index).swap(productionLine.front());
			setIC(totalDedicatedIC);
		}
		productionLineLock.unlock();
	}

	void Production::setProductionItemAtMinPriority(const unsigned short index)
	{
		productionLineLock.lock();
		if (index < productionLine.size() - 1)
		{
			productionLine.at(index).swap(productionLine.back());  
			setIC(totalDedicatedIC);
		}
		productionLineLock.unlock();
	}

	void Production::removeProductionItem(const unsigned short index)
	{
		productionLineLock.lock();
		if (index < productionLine.size()) 
		{
			if (productionLine.at(index)->getUnit().getType() == Unit::Building && productionLine.at(index)->getTarget() != -1)
				Region::get(productionLine.at(index)->getTarget()).dequeueBuilding(productionLine.at(index)->getUnit().getName());
			productionLine.erase(productionLine.begin() + index);
			setIC(totalDedicatedIC); 
		}
		productionLineLock.unlock();
	}

	void Production::deployUnit(const unsigned short index, const unsigned short targetRegion)
	{
		deploymentQueueLock.lock();
		if (index < deploymentQueue.size() - 1 && deploymentQueue.at(index) && deploy(*deploymentQueue.at(index), targetRegion))
			deploymentQueue.erase(deploymentQueue.begin() + index);
		deploymentQueueLock.unlock();
	}

	void Production::addProductionItem(const string& element, const unsigned short targetRegion)
	{
		if (Unit::exists(element) && ResourceDistributor::get(tag).getManpowerAmount() >= Unit::get(element).getRequiredManpower())
		{
			productionLineLock.lock();
			productionLine.emplace_back(new ProductionItem(Unit::get(element),
				Unit::get(element).getProductionDays(tag),
				targetRegion));
			productionLineLock.unlock();

			if (Unit::get(element).getType() == Unit::Building) Region::get(targetRegion).enqueueBuilding(element);

			ResourceDistributor::get(tag).changeManpowerAmount((-1) * Unit::get(element).getRequiredManpower());
			setIC(totalDedicatedIC);
		}
	}

	const float Production::setIC(float IC)
	{
		if (IC < 0.0f) IC = 0.0f;

		totalDedicatedIC = IC;
		productionLineLock.lock();
		for (auto it = productionLine.begin(), end = productionLine.end(); it != end; ++it)
		{
			const float requiredIC((*it)->getUnit().getRequiredIC(tag));
			(*it)->setIC(IC / requiredIC);
			IC > requiredIC ? IC -= requiredIC : IC = 0.0f;
		}
		productionLineLock.unlock();
		return IC;
	}

	void Production::update()
	{
		productionLineLock.lock();
		for (auto it(productionLine.begin()), end(productionLine.end()); it != end; ++it)
			if (*it)
			{
				(*it)->updateProductionDays(tag);
				if ((*it)->produce())
				{
					if (deploy(**it, (*it)->getTarget()));
					else if ((*it)->getUnit().isDelayDeployable()) deploymentQueue.push_back(std::move(*it));

					if ((*it)->getUnit().getExperienceReward())
						Research::get(tag).addExperienceRewards(*(*it)->getUnit().getExperienceReward(), 1.0f);
					it = productionLine.erase(it);
				}
			}
		productionLineLock.unlock();
	}
	
	const bool Production::deploy(const ProductionItem& item, const unsigned short targetRegion)
	{
		if (Region::exists(targetRegion) && Region::get(targetRegion).getController() == tag)
		{
			switch (item.getUnit().getType())
			{
			case Unit::Building:
				Region::get(targetRegion).build(item.getUnit());
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
			return (TimeSystem::getCurrentDate() + (((100.0f - completionPercentage) / 100.0f) * (1.0f / dedicatedICPercentage) * productionDays) * 24);
		else return Date::NEVER;
	}

	Production::ProductionItem& Production::ProductionItem::updateProductionDays(const Tag& tag)
	{
		productionDays = unit.getProductionDays(tag);
		return *this;
	}

	const bool Production::ProductionItem::produce()
	{
		if (dedicatedICPercentage > 0.0f) completionPercentage = completionPercentage + (100.0f / (productionDays * (1.0f / dedicatedICPercentage)));
		return completionPercentage >= 100.0f;
	}
};