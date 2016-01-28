#include "Nation.h"

#include "Diplomacy.h"
#include "ResourceDistributor.h"
#include "LeadershipDistributor.h"

#include <map>
#include <fstream>
#include <sstream>

const std::string FLAGDIR = "interface visual/flags/", NATIONSTAGDEFDIR = "countries.csv", NATIONSDEFDIR = "countries.txt";

using namespace std;

namespace bEnd
{
	void Nation::resetIncomeGlobal()
	{
		for (auto it = nations.begin(), end = nations.end(); it != end; ++it)
			ResourceDistributor::getResourceDistributor(it->first).resetIncome();
	}

	void Nation::updateGlobal()
	{
		resetIncomeGlobal();
		//Region::generateResourcesGlobal(nations);
		Diplomacy::trade();
		for (auto it = nations.begin(), end = nations.end(); it != end; ++it)
		{
			ResourceDistributor::getResourceDistributor(it->first).update();
			LeadershipDistributor::getLeadershipDistributor(it->first).update();
		}
	}

	void Nation::loadNations()
	{
		nations.clear();
		ifstream file(NATIONSTAGDEFDIR);
		while (!file.eof())
		{
			char* line = new char[1 << 10];
			file.getline(line, 1 << 10);
			if (line[0] != '\0')
			{
				stringstream lineStream(line);
				Nation buffer;
				lineStream.getline(line, 1 << 10, ';');
				buffer.name = line;
				lineStream.getline(line, 1 << 10, '\0');
				buffer.tag = line;

				nations.insert(make_pair(buffer.tag, buffer));
			}
			delete line;
		}
		file.close();
		file.open(NATIONSDEFDIR);
		Continent continentBuffer = Europe;
		while (!file.eof())
		{
			char* line = new char[1 << 10];
			file.getline(line, 1 << 10);
			if (line[0] != '\0')
				if (line[0] == '#')
				{
					if (line == "#Europe")continentBuffer = Europe;
					else if (line == "#Asia")continentBuffer = Asia;
					else if (line == "#Africa")continentBuffer = Africa;
					else if (line == "#NorthAmerica")continentBuffer = NorthAmerica;
					else if (line == "#SouthAmerica")continentBuffer = SouthAmerica;
				}
				else
				{
					string stringBuffer;
					stringBuffer.push_back(line[0]);
					stringBuffer.push_back(line[1]);
					stringBuffer.push_back(line[2]);
					for (auto it = nations.begin(), end = nations.end(); it != end; ++it)
						if (it->first == stringBuffer)
						{
							it->second.continent = continentBuffer;
							stringstream strStream(line);
							getline(strStream, stringBuffer, '"');
							getline(strStream, stringBuffer, '"');
							fstream countryFile(stringBuffer);
							while (!countryFile.eof())
							{
								countryFile.getline(line, 1 << 10);
								if (string(line) == "major = yes")
								{
									it->second.major = true;
									continue;
								}
								strStream.str(line);
								getline(strStream, stringBuffer, '{');
								if (stringBuffer == "color = ")
								{
									getline(strStream, stringBuffer, '}');
									strStream.str(stringBuffer);
									strStream >> stringBuffer;
								}
								else
								{
								}
							}
						}
				}
			delete line;
		}
	}
}


