#include "Nation.h"

#include "../Backend/FileProcessor.h"

namespace fEnd
{
	std::mutex Nation::nationsLock;
	std::unordered_map<bEnd::Tag, Nation> Nation::nations;

	void Nation::loadFromFile(const std::string& path)
	{
		bEnd::FileProcessor source(path);
		if (!source.isOpen()) return;
		for (auto& it : source.getStatements())
			if (it.lValue == "color")
			{
				color.r = std::stoi(it.rStrings.at(0));
				color.g = std::stoi(it.rStrings.at(1));
				color.b = std::stoi(it.rStrings.at(2));
			}
	}

	void Nation::loadNations()
	{
		{
			std::ifstream source("nations.csv");
			if (!source.is_open()) return;

			nationsLock.lock();
			while (!source.eof())
			{
				std::string buffer;
				std::getline(source, buffer, '\n');
				std::stringstream line(buffer);
				std::getline(line, buffer, ';');
				std::getline(line, nations[buffer].name, ';');
			}
			nationsLock.unlock();
		}

		bEnd::FileProcessor source("nations.txt");
		if (!source.isOpen()) return;

		nationsLock.lock();
		for (auto& it : source.getStatements())
		{
			Continent currentContinent;
			if (it.lValue == "Europe") currentContinent = Continent::Europe;
			else if (it.lValue == "Asia") currentContinent = Continent::Asia;
			else if (it.lValue == "Africa") currentContinent = Continent::Africa;
			else if (it.lValue == "Oceania") currentContinent = Continent::Oceania;
			else if (it.lValue == "NorthAmerica") currentContinent = Continent::NorthAmerica;
			else if (it.lValue == "SouthAmerica") currentContinent = Continent::SouthAmerica;

			for (auto it1 : it.rStatements)
			{
				nations.at(it1.lValue).continent = currentContinent;
				nations.at(it1.lValue).flag.loadFromFile("resources/flags/" + it1.lValue + ".png");
				nations.at(it1.lValue).loadFromFile(it1.rStrings.front());
			}
		}
		nationsLock.unlock();
	}
}