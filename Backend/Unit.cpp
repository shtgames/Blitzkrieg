#include "Unit.h"

namespace bEnd
{
	const float Unit::getRequiredIC(const std::map<std::string, float>&)const
	{
		return 0.0f;
	}
	const unsigned short Unit::getProductionDays(const std::map<std::string, float>& practicalAndTheoreticalKnowledge) const
	{
		return 0;
	}
	const bool Unit::loadFromFile(const std::string & file)
	{
		return false;
	}
}