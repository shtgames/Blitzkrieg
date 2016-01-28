#include "Politics.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<Politics>> Politics::politics;

	Politics::Politics()
	{
	}

	void Politics::update()
	{
	}
	const bool Politics::loadFronFile(ifstream &)
	{
		return false;
	}
}