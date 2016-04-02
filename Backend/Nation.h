#pragma once

#include "Tag.h"
#include "FileProcessor.h"

namespace bEnd
{
	class Nation final
	{
	public:
		static void loadFromSave(const FileProcessor::Statement& source);
		static Tag player;
	};
}