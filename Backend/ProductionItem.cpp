#include "ProductionItem.h"

#include "TimeSystem.h"

namespace bEnd
{

	ProductionItem::ProductionItem() : productionDays(0), requiredIC(0.0f) {}

	ProductionItem::ProductionItem(const unsigned short& _Dval, const float& _ICval) : productionDays(_Dval), requiredIC(_ICval) {}

	void ProductionItem::setIC(const float& _dedicatedReourceAmount)
	{
		if (_dedicatedReourceAmount >= 0.0f) dedicatedIC = _dedicatedReourceAmount;
	}

	const double& ProductionItem::getCompletionPercenTage()
	{
		return completionPercenTage;
	}

	Date ProductionItem::getComlpetionDate()
	{
		if (dedicatedIC > 0.0f)
			return (TimeSystem::getCurrentDate() + Date(((100.0f - completionPercenTage) * productionDays * (requiredIC / dedicatedIC)) * 24));
		else return Date();
	}

	const float ProductionItem::getDedicatedICPercenTage()
	{
		if (requiredIC > 0.0f) return 100.0f * (dedicatedIC / requiredIC); else return 0.0f;
	}

	void ProductionItem::update()
	{
		if (dedicatedIC > 0.0f) completionPercenTage += (100.0f / (productionDays * (requiredIC / dedicatedIC)));
	}

}