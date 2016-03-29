#include "SaveGame.h"

#include <algorithm>
#include <stack>
#include <cctype>

namespace bEnd 
{
	std::unordered_map<std::string, std::function<void(const SaveGame::Statement&)>> SaveGame::previewSyntaxMap, SaveGame::loadSyntaxMap;

	const bool SaveGame::open(const std::string& filePath)
	{
		std::ifstream::open(filePath);
		if (std::ifstream::is_open())
			return !(statementsVectorNeedsUpdate = false);
		return false;
	}

	void SaveGame::preview()
	{
		processFile(0);
	}

	void SaveGame::load()
	{
		processFile(1);
	}

	void SaveGame::addSyntax(const std::string& statement, const std::function<void(const Statement&)>& codeBlockProcessor, const bool validityCase)
	{
		auto& target(validityCase ? loadSyntaxMap : previewSyntaxMap);
		target.count(statement) ? target.at(statement) = codeBlockProcessor : target.emplace(std::make_pair(statement, codeBlockProcessor));
	}

	void SaveGame::addSyntax(const std::string& statement, std::function<void(const Statement&)>&& codeBlockProcessor, const bool validityCase)
	{
		auto& target(validityCase ? loadSyntaxMap : previewSyntaxMap);
		target.count(statement) ? target.at(statement) = std::move(codeBlockProcessor) :
			target.emplace(std::make_pair(statement, std::move(codeBlockProcessor)));
	}

	const SaveGame::Statement SaveGame::getNextStatement(std::istream& source)
	{
		Statement returnValue;

		skipWhitespace(source);
		std::getline(source, returnValue.lValue, '=');
		returnValue.lValue.erase(std::remove(returnValue.lValue.begin(), returnValue.lValue.end(), '\t'),
			returnValue.lValue.end());
		returnValue.lValue.erase(std::remove(returnValue.lValue.begin(), returnValue.lValue.end(), ' '),
			returnValue.lValue.end());

		skipWhitespace(source);		

		char input;
		source >> input;

		if (input != '{')
		{
			std::string buffer{input};
			source >> buffer;
			returnValue.rStrings.emplace_back(std::move(buffer));
			skipWhitespace(source);
			return returnValue;
		}
		
		while (!source.eof())
		{
			std::string buffer;
			skipWhitespace(source);
			const auto pos = source.tellg();
			std::getline(source, buffer, '\n');
			if (!buffer.empty() && buffer.front() == '}') break;
			source.seekg(pos);
			if (buffer.find_first_of('=') != std::string::npos)
				returnValue.rStatements.emplace_back(std::move(getNextStatement(source)));
			else
			{
				source >> buffer;
				returnValue.rStrings.emplace_back(std::move(buffer));
				skipWhitespace(source);
			}
		}

		return returnValue;
	}

	void SaveGame::processFile(const bool asPreviewOrLoad)
	{
		auto& syntaxMap(asPreviewOrLoad ? loadSyntaxMap : previewSyntaxMap);
		if (statementsVectorNeedsUpdate)
		{
			if (!std::ifstream::is_open()) return;

			while (!std::ifstream::eof())
			{
				const Statement statement(std::move(getNextStatement(*this)));
				if (syntaxMap.count(statement.lValue))
					syntaxMap.at(statement.lValue)(statement);
			}
			std::ifstream::clear();
			std::ifstream::seekg(0, std::ios::beg);
			statementsVectorNeedsUpdate = false;
		}
		else for (auto it = statements.begin(), end = statements.end(); it != end; ++it)
			if (syntaxMap.count(it->lValue)) syntaxMap.at(it->lValue)(*it);
	}

	void SaveGame::skipWhitespace(std::istream& source)
	{
		char input;
		do 
		{
			source >> input;
			if (input != ' ' && input != '\n' && input != '\t')
			{
				if (input == '#') std::getline(source, std::string(), '\n');
				else break;
			}
		} while (!source.eof());
		if (!source.eof()) source.seekg(int(source.tellg()) - 1);
	}
}
