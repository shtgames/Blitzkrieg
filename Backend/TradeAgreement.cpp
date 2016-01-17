#include "TradeAgreement.h"

namespace bEnd
{
	TradeAgreement::TradeAgreement(const std::map<Resource, float>& _values, const Tag& _source, const Tag& _target)
		: tradeValues(_values)
	{
		updateTradeCosts(_source, _target);
	}

	void TradeAgreement::updateTradeCosts(const Tag& _source, const Tag& _target)
	{

	}

	const std::map<Resource, float>& TradeAgreement::getTradeValues()const
	{
		return tradeValues;
	}

	const std::map<Resource, float> TradeAgreement::flipTradeValues()const
	{
		std::map<Resource, float> returnValue;
		for (auto it = tradeValues.begin(), end = tradeValues.end(); it != end; ++it)
		{
			returnValue.emplace(std::make_pair(it->first, -it->second));
		}
		return returnValue;
	}
}