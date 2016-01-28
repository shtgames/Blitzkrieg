#include "Diplomacy.h"

#include "ResourceDistributor.h"

namespace bEnd
{
	std::unordered_map<Tag, std::map<Tag, Diplomacy::warGoal>> Diplomacy::war;
	std::unordered_map<Tag, std::pair<std::set<Tag>, std::set<Tag>>> Diplomacy::guarantee, Diplomacy::embargo, Diplomacy::transitRights;
	std::unordered_map<Tag, std::set<Tag>> Diplomacy::alliance, Diplomacy::nonAggressionPact;
	std::map<std::pair<Tag, Tag>, std::vector<TradeAgreement>> Diplomacy::tradeAgreements;
	std::unordered_map<std::pair<Tag, Tag>, short> Diplomacy::relations;

	void Diplomacy::trade()
	{
		for (auto it = tradeAgreements.begin(), end = tradeAgreements.end(); it != end; ++it)
			for (auto it1 = it->second.begin(), end1 = it->second.end(); it1 != end1; ++it1)
			{
				const Tag& source = it->first.first, target = it->first.second;
				if (ResourceDistributor::getResourceDistributor(source).contains(it1->getTradeValues()) && ResourceDistributor::getResourceDistributor(target).contains(it1->flipTradeValues()))
				{
					ResourceDistributor::getResourceDistributor(source).transferResourcesFromTrade(it1->getTradeValues());
					ResourceDistributor::getResourceDistributor(target).transferResourcesFromTrade(it1->flipTradeValues());
				}
			}
	}

	void Diplomacy::createAliiance(const Tag& _source, const Tag& _target)
	{
		alliance[_source].emplace(_target);
		alliance[_target].emplace(_source);
		changeRelations(_source, _target, 30);
	}

	void Diplomacy::dissolveAlliance(const Tag& _source, const Tag& _target)
	{
		alliance[_source].erase(_target);
		alliance[_target].erase(_source);
		changeRelations(_source, _target, -40);
	}

	void Diplomacy::declareWar(const Tag& _source, const Tag& _target)
	{
		declareLimitedWar(_source, _target);
		for (auto it = alliance[_source].begin(), end = alliance[_source].end(); it != end; ++it)
			declareLimitedWar(*it, _target);
		for (auto it = alliance[_target].begin(), end = alliance[_target].end(); it != end; ++it)
			declareLimitedWar(*it, _source);
	}

	void Diplomacy::declareLimitedWar(const Tag& _source, const Tag& _target)
	{
		war[_source].emplace(std::make_pair(_target, Conquer));
		war[_target].emplace(std::make_pair(_source, Conquer));
		setRelations(_source, _target, -200);
	}

	void Diplomacy::peace(const Tag& _source, const Tag& _target)
	{
		war[_source].erase(_target);
		war[_target].erase(_source);
		setRelations(_source, _target, -30);
	}

	void Diplomacy::imposeEmbargo(const Tag& _source, const Tag& _target)
	{
		tradeAgreements.erase(std::make_pair(_source, _target));
		embargo[_source].first.emplace(_target);
		embargo[_target].second.emplace(_source);
		changeRelations(_source, _target, -50);
	}

	void Diplomacy::liftEmbargo(const Tag& _source, const Tag& _target)
	{
		embargo[_source].first.erase(_target);
		embargo[_target].second.erase(_source);
		changeRelations(_source, _target, 30);
	}

	void Diplomacy::createNonAggressionPact(const Tag& _source, const Tag& _target)
	{
		nonAggressionPact[_source].emplace(_target);
		nonAggressionPact[_target].emplace(_source);
		changeRelations(_source, _target, 15);
	}

	void Diplomacy::breakNonAggressionPact(const Tag& _source, const Tag& _target)
	{
		nonAggressionPact[_source].erase(_target);
		nonAggressionPact[_target].erase(_source);
		changeRelations(_source, _target, -30);
	}

	void Diplomacy::expireNonAggressionPact(const Tag& _source, const Tag& _target)
	{
		nonAggressionPact[_source].erase(_target);
		nonAggressionPact[_target].erase(_source);
	}

	void Diplomacy::declareGuarantee(const Tag& _source, const Tag& _target)
	{
		guarantee[_source].first.emplace(_target);
		guarantee[_target].second.emplace(_source);
		changeRelations(_source, _target, 35);
	}

	void Diplomacy::revokeGuarantee(const Tag& _source, const Tag& _target)
	{
		guarantee[_source].first.erase(_target);
		guarantee[_target].second.erase(_source);
		changeRelations(_source, _target, -45);
	}

	void Diplomacy::giveTransitRights(const Tag& _source, const Tag& _target)
	{
		transitRights[_source].first.emplace(_target);
		transitRights[_target].second.emplace(_source);
		changeRelations(_source, _target, 10);
	}

	void Diplomacy::revokeTransitRights(const Tag& _source, const Tag& _target)
	{
		transitRights[_source].first.erase(_target);
		transitRights[_target].second.erase(_source);
		changeRelations(_source, _target, -20);
	}

	void Diplomacy::createTradeAgreement(const Tag& _source, const Tag& _target, const TradeAgreement& _TradeAgreement)
	{
		tradeAgreements[std::make_pair(_source, _target)].push_back(_TradeAgreement);
		changeRelations(_source, _target, 5);
	}

	void Diplomacy::dissolveTradeAgreement(const Tag& _source, const Tag& _target, const std::vector<TradeAgreement>::iterator& _TradeAgreement)
	{
		tradeAgreements[std::make_pair(_source, _target)].erase(_TradeAgreement);
		changeRelations(_source, _target, -10);
	}

	void Diplomacy::changeRelations(const Tag& _source, const Tag& _target, const short& _value)
	{
		if (_value > 0)
		{
			if (relations[std::make_pair(_source, _target)] + _value > 200)
				relations[std::make_pair(_source, _target)] = 200;
			else
				relations[std::make_pair(_source, _target)] += _value;
		}
		else
		{
			if (relations[std::make_pair(_source, _target)] + _value < -200)
				relations[std::make_pair(_source, _target)] = -200;
			else
				relations[std::make_pair(_source, _target)] += _value;
		}
	}

	void Diplomacy::setRelations(const Tag& _source, const Tag& _target, const short& _value)
	{
		if (_value >= -200 && _value <= 200)
			relations[std::make_pair(_source, _target)] = _value;
	}
}