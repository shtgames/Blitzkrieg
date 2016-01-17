#include "Production.h"

namespace bEnd
{
	Production::Production()
		: baseIC(0), availableIC(0), wastedIC(0)
	{
		resources[Energy];
		resources[Metal];
		resources[CrudeOil];
		resources[RareMaterials];
		resources[Supplies];
		resources[Fuel];
		resources[Money];
	}

	const bool Production::contains(const std::map<Resource, float>& _rVal)const
	{
		for (auto it = _rVal.begin(), end = _rVal.end(); it != end; ++it)
			if (resources.at(it->first).first < it->second) return false;
		return true;
	}

	void Production::resetIncome()
	{
		baseIC = 0;
		availableIC = 0;
		wastedIC = 0;

		for (auto it = resources.begin(), end = resources.end(); it != end; ++it)
			for (auto it1 = it->second.second.begin(), end1 = it->second.second.end(); it1 != end1; ++it1)
				it1->second = 0.0f;
	}

	void Production::transferResourcesFromRegion(const std::map<Resource, float>& _source, const bool& _ownerAndControllerAreTheSame, const bool& _hasCore, const OccupationPolicy& _OccupationPolicy)
	{
		float modifier = 1.0f;
		if (_hasCore && _ownerAndControllerAreTheSame) modifier = 1.0f;
		else if (_ownerAndControllerAreTheSame) modifier = 0.5f;
		else modifier = _OccupationPolicy.getResourceModifier();

		resources[Energy].first += _source.at(Energy) * modifier;
		resources[Energy].second[generated] += _source.at(Energy) * modifier;

		resources[Metal].first += _source.at(Metal) * modifier;
		resources[Metal].second[generated] += _source.at(Metal) * modifier;

		resources[RareMaterials].first += _source.at(RareMaterials) * modifier;
		resources[RareMaterials].second[generated] += _source.at(RareMaterials) * modifier;

		resources[CrudeOil].first += _source.at(CrudeOil) * modifier;
		resources[CrudeOil].second[generated] += _source.at(CrudeOil) * modifier;
	}

	void Production::transferManpowerFromRegion(const float& _manpower, const bool& _ownerAndControllerAreTheSame, const bool& _hasCore, const OccupationPolicy& _OccupationPolicy)
	{
		if (_hasCore && _ownerAndControllerAreTheSame)
			manpower += _manpower;
		else if (_ownerAndControllerAreTheSame)
			manpower += _manpower * 0.5f;
		else
			manpower += _manpower * _OccupationPolicy.getMPModifier();
	}

	void Production::transferResourcesFromTrade(const std::map<Resource, float>& _source)
	{
		for (std::map<Resource, float>::const_iterator it = _source.begin(), end = _source.end(); it != end; ++it)
			if (resources[it->first].first + it->second > 0.0f)
			{
				resources[it->first].first += it->second;
				resources[it->first].second[traded] += it->second;
			}
	}

	void Production::transferIC(const unsigned short& _IC, const bool& _ownerAndControllerAreTheSame, const bool& _hasCore, const OccupationPolicy& _OccupationPolicy)
	{
		if (_hasCore && _ownerAndControllerAreTheSame) baseIC += _IC * 1.0f;
		else if (_ownerAndControllerAreTheSame) baseIC += _IC * 0.5f;
		else baseIC += _IC * _OccupationPolicy.getICModifier();
	}

	void Production::calculateIC()
	{
		float usedIC = 0.0f;
		for (auto it = ICDistribution.begin(), end = ICDistribution.end(); it != end; ++it)
		{
			if (it->second > ICNeeds[it->first]) { wastedIC += it->second - ICNeeds[it->first]; usedIC += ICNeeds[it->first]; }
			else { usedIC += it->second; }
		}
		if (baseIC > 0.0f)
		{
			float resourceBottleneck = 1.0f;
			if (resources[Energy].first < (baseIC * 2.0f)) // 1 x IC = 2 x Energy + 1 x Metal + 0.5 x Rare Materials;
			{
				resourceBottleneck = (resources[Energy].second[generated] + resources[Energy].second[traded]) / (baseIC * 2.0f);
			}

			if (resources[Metal].first < (baseIC * 1.0f))
			{
				if (((resources[Metal].second[generated] + resources[Metal].second[traded]) / (baseIC * 1.0f)) < resourceBottleneck)
					resourceBottleneck = (resources[Metal].second[generated] + resources[Metal].second[traded]) / (baseIC * 1.0f);
			}

			if (resources[RareMaterials].first < (baseIC * 0.5f))
			{
				if (((resources[RareMaterials].second[generated] + resources[RareMaterials].second[traded]) / (baseIC * 0.5f)) < resourceBottleneck)
					resourceBottleneck = ((resources[RareMaterials].second[generated] + resources[RareMaterials].second[traded]) / (baseIC * 0.5f));
			}
			availableIC = resourceBottleneck * baseIC; // * other modifiers;

			resources[Energy].first -= (usedIC > availableIC ? availableIC : usedIC) * 2.0f;
			resources[Energy].second[used] = (usedIC > availableIC ? availableIC : usedIC) * (-2.0f);

			resources[Metal].first -= (usedIC > availableIC ? availableIC : usedIC) * 1.0f;
			resources[Metal].second[used] = (usedIC > availableIC ? availableIC : usedIC) * (-1.0f);

			resources[RareMaterials].first -= (usedIC > availableIC ? availableIC : usedIC) * 0.5f;
			resources[RareMaterials].second[used] = (usedIC > availableIC ? availableIC : usedIC) * (-0.5f);
		}
	}

	void Production::resourceConversions()
	{
		if (baseIC > 0.0f)
		{
			float resourceBottleneck = 1.0f;
			if (resources[Energy].first < 0.5f * availableIC)//The Amount of Energy Converted to Crude Oil = (10 x Available IC / 20) / The level of “Coal to Oil Conversion” Technology
				resourceBottleneck = resources[Energy].first / (0.5f * availableIC);

			resources[Energy].first -= resourceBottleneck * (0.5f * availableIC);
			resources[Energy].second[convertedIntoCrudeOil] = resourceBottleneck * (-0.5f * availableIC);

			resources[CrudeOil].first += resourceBottleneck * (availableIC / 20.0f);
			resources[CrudeOil].second[convertedFromEnergy] = resourceBottleneck * (availableIC / 20.0f);

			resourceBottleneck = 1.0f;

			if (resources[CrudeOil].first < availableIC / 2.0f)//Total Fuel Production = (Base IC / 2) x(1 + (PercenTage of The “Oil Refining” Technology - 0, 1))
				resourceBottleneck = resources[CrudeOil].first / (availableIC / 2.0f);

			resources[CrudeOil].first -= resourceBottleneck * (availableIC / 2.0f);
			resources[CrudeOil].second[convertedIntoFuel] = resourceBottleneck * (availableIC / -2.0f);

			resources[Fuel].first += resourceBottleneck * (availableIC / 2.0f);
			resources[Fuel].second[convertedFromCrudeOil] = resourceBottleneck * (availableIC / 2.0f);
		}
	}


	void Production::distributeIC()
	{
		ICToUpgrades();
		ICToReinforcement();
		ICToSupplies();
		ICToProductionLine();
		ICToConsumerGoods();
		ICToLendLease();
	}

	void Production::ICToUpgrades()
	{

	}

	void Production::ICToReinforcement()
	{

	}

	void Production::ICToSupplies()
	{

	}

	void Production::ICToProductionLine()
	{

	}

	void Production::ICToConsumerGoods()
	{

	}

	void Production::ICToLendLease()
	{

	}


	void Production::calculateMoney()
	{
		if (baseIC > 0.0f)
		{
			resources[Money].first += baseIC / 20.0f; // Money Gain = (“Base IC” / 20) x ( 1 + ( "Economic Laws" PercenTage of Money + "Education Investment Laws" PercenTage of Money + "Minister traits" PercenTage of Money + “Event” PercenTage of Money ) )
			resources[Money].second[generated] = baseIC / 20.0f;
		}
	}

	void Production::increaseProductionItemPriority(const unsigned short& _index)
	{
		if (_index >= 1 && _index < productionLine.size())
		{
			ProductionItem* buffer = productionLine.at(_index - 1);
			productionLine.at(_index - 1) = productionLine.at(_index);
			productionLine.at(_index) = buffer;
		}
	}

	void Production::decreaseProductionItemPriority(const unsigned short& _index)
	{
		if (_index >= 0 && _index < productionLine.size() - 1)
		{
			ProductionItem* buffer = productionLine.at(_index + 1);
			productionLine.at(_index + 1) = productionLine.at(_index);
			productionLine.at(_index) = buffer;
		}
	}

	void Production::setProductionItemAtMaxPriority(const unsigned short& _index)
	{
		if (_index >= 1 && _index < productionLine.size())
		{
			ProductionItem* buffer = productionLine.at(0);
			productionLine.at(0) = productionLine.at(_index);
			productionLine.at(_index) = buffer;
		}
	}

	void Production::setProductionItemAtMinPriority(const unsigned short& _index)
	{
		if (_index >= 0 && _index < productionLine.size() - 1)
		{
			ProductionItem* buffer = productionLine.at(productionLine.size() - 1);
			productionLine.at(productionLine.size() - 1) = productionLine.at(_index);
			productionLine.at(_index) = buffer;
		}
	}

	void Production::removeProductionItem(const unsigned short& _index)
	{
		if (_index >= 0 && _index < productionLine.size())
		{
			std::vector<ProductionItem*> buffer;
			for (unsigned short i = 0, end = productionLine.size(); i < end; i++)
				if (i != _index) buffer.push_back(productionLine.at(i));
				else delete productionLine.at(i);
		}
	}

	void Production::addProductionItem(ProductionItem* _element)
	{
		productionLine.push_back(_element);
	}

	void Production::update()
	{
		calculateIC();
		resourceConversions();
		distributeIC();
		calculateMoney();
	}
}