#ifndef PRODUCTION_ITEM_BACKEND
#define PRODUCTION_ITEM_BACKEND

#include "Date.h"

namespace bEnd
{
	class ProductionItem
	{
	public:
		ProductionItem();
		ProductionItem(const unsigned short&, const float&);
		void setIC(const float&);
		const double& getCompletionPercenTage();
		Date getComlpetionDate();
		const float getDedicatedICPercenTage();
		void update();
		virtual void onCompletion() = 0;
	private:
		const unsigned short productionDays;
		double completionPercenTage;
		const float requiredIC;
		float dedicatedIC;
	};
}

#endif