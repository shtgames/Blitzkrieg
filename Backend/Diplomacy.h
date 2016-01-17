#ifndef DIPLOMACY_BACKEND
#define DIPLOMACY_BACKEND

#include "Tag.h"
#include "Resources.h"
#include "TradeAgreement.h"

#include <map>
#include <set>

namespace bEnd
{

	class Diplomacy final
	{
	 public:

		enum warGoal
		{
			Conquer
		};

		 static void trade();
		 static void createAliiance(const Tag&, const Tag&);
		 static void dissolveAlliance(const Tag&, const Tag&);
		 static void declareWar(const Tag&, const Tag&);
		 static void declareLimitedWar(const Tag&, const Tag&);
		 static void peace(const Tag&, const Tag&);
		 static void imposeEmbargo(const Tag&, const Tag&);
		 static void liftEmbargo(const Tag&, const Tag&);
		 static void createNonAggressionPact(const Tag&, const Tag&);
		 static void breakNonAggressionPact(const Tag&, const Tag&);
		 static void expireNonAggressionPact(const Tag&, const Tag&);
		 static void declareGuarantee(const Tag&, const Tag&);
		 static void revokeGuarantee(const Tag&, const Tag&);
		 static void giveTransitRights(const Tag&, const Tag&);
		 static void revokeTransitRights(const Tag&, const Tag&);
		 static void createTradeAgreement(const Tag&, const Tag&, const TradeAgreement&);
		 static void dissolveTradeAgreement(const Tag&, const Tag&, const std::vector<TradeAgreement>::iterator&);

		 static const std::unordered_map<Tag, std::map<Tag, warGoal>>&                  getWars() { return war; }
		 static const std::unordered_map<Tag, std::pair<std::set<Tag>, std::set<Tag>>>& getGuarantess() { return guarantee; }
		 static const std::unordered_map<Tag, std::pair<std::set<Tag>, std::set<Tag>>>& getEmbargoes() { return embargo; }
		 static const std::unordered_map<Tag, std::set<Tag>>&                           getAlliances() { return alliance; }
		 static const std::unordered_map<Tag, std::set<Tag>>&                           getNonAggressionPacts() { return nonAggressionPact; }
		 static const std::unordered_map<Tag, std::pair<std::set<Tag>, std::set<Tag>>>& getTransitRights() { return transitRights; }
		 static const std::map<std::pair<Tag, Tag>, std::vector<TradeAgreement>>&       gettradeAgreements() { return tradeAgreements; }
		 static const std::unordered_map<std::pair<Tag, Tag>, short>&                   getRelations() { return relations; }
	 private:
		 static void changeRelations(const Tag&, const Tag&, const short&);
		 static void setRelations(const Tag&, const Tag&, const short&);

		 static std::unordered_map<Tag, std::map<Tag, warGoal>>                  war;
		 static std::unordered_map<Tag, std::pair<std::set<Tag>, std::set<Tag>>> guarantee, embargo, transitRights;
		 static std::unordered_map<Tag, std::set<Tag>>                           alliance, nonAggressionPact;
		 static std::map<std::pair<Tag, Tag>, std::vector<TradeAgreement>>       tradeAgreements;
		 static std::unordered_map<std::pair<Tag, Tag>, short>                   relations;

	 friend class economy;
	};
}

#endif