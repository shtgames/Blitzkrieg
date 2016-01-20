#include "Region.h"

#include "Production.h"

namespace bEnd
{
	void Production::increaseProductionItemPriority(const unsigned short index)
	{
		if (index < productionLine.size() - 1 && index != 0)
			productionLine.at(index).swap(productionLine.at(index - 1));
	}

	void Production::decreaseProductionItemPriority(const unsigned short index)
	{
		if (index < productionLine.size() - 1 && index != productionLine.size() - 2)
			productionLine.at(index).swap(productionLine.at(index + 1));
	}

	void Production::setProductionItemAtMaxPriority(const unsigned short index)
	{
		if (index < productionLine.size() - 1)
			productionLine.at(index).swap(productionLine.front());
	}

	void Production::setProductionItemAtMinPriority(const unsigned short index)
	{
		if (index < productionLine.size() - 1)
			productionLine.at(index).swap(productionLine.back());
	}

	void Production::removeProductionItem(const unsigned short index)
	{
		if (index < productionLine.size() - 1)
			productionLine.erase(productionLine.begin() + index);
	}

	void Production::addProductionItem(std::unique_ptr<ProductionItem>&& element)
	{
		productionLine.emplace_back(std::move(element));
	}

	const float Production::setIC(const float IC)
	{
		for (auto it = productionLine.begin(), end = productionLine.end(); it != end; ++it)
			(*it);// ->setIC(IC -= (*it)->);
	}

	void Production::update()
	{
		for (auto it = productionLine.begin(), end = productionLine.end(); it != end; ++it)
			(*it)->update();
	}
	
	const bool Production::loadFromFile(ifstream& file)
	{
		return false;
	}
};