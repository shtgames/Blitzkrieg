#include "Politics.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<Politics>> Politics::politics;

	void Politics::update()
	{
		//...
	}

	const bool Politics::loadFromFile(ifstream& file)
	{
		return false;
	}
}