#ifndef TRADE_AGREEMENT_BACKEND
#define TRADE_AGREEMENT_BACKEND

#include "Tag.h"
#include "Resources.h"

#include <map>

namespace bEnd
{
	class TradeAgreement final
	{
	public:
		TradeAgreement(const std::map<Resource, float>&, const Tag&, const Tag&);
		TradeAgreement(const TradeAgreement&) = default;
		TradeAgreement(TradeAgreement&&) = default;
		TradeAgreement() = default;
		~TradeAgreement() = default;

		void updateTradeCosts(const Tag&, const Tag&);
		const std::map<Resource, float>& getTradeValues()const;
		const std::map<Resource, float> flipTradeValues()const;
	private:
		std::map<Resource, float> tradeValues;
	};
}

#endif