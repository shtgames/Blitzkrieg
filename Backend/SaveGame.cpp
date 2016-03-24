#include "SaveGame.h"

#include <algorithm>
#include <stack>
#include <cctype>

namespace bEnd 
{
	std::unordered_map<std::string, std::function<void(const std::stringstream&)>> SaveGame::previewSyntaxMap, SaveGame::loadSyntaxMap;

	const bool SaveGame::open(const std::string& filePath)
	{
		std::ifstream::open(filePath);
		return std::ifstream::is_open();
	}

	void SaveGame::preview()
	{
		processFile(0);
	}

	void SaveGame::load()
	{
		processFile(1);
	}

	void SaveGame::addSyntax(const std::string& statement, const std::function<void(const std::stringstream&)>& codeBlockProcessor, const bool validityCase)
	{
		auto& target(validityCase ? loadSyntaxMap : previewSyntaxMap);
		target.count(statement) ? target.at(statement) = codeBlockProcessor : target.emplace(std::make_pair(statement, codeBlockProcessor));
	}

	void SaveGame::addSyntax(const std::string& statement, std::function<void(const std::stringstream&)>&& codeBlockProcessor, const bool validityCase)
	{
		auto& target(validityCase ? loadSyntaxMap : previewSyntaxMap);
		target.count(statement) ? target.at(statement) = std::move(codeBlockProcessor) :
			target.emplace(std::make_pair(statement, std::move(codeBlockProcessor)));
	}

	std::pair<std::string, std::stringstream> SaveGame::getNextStatement(std::istream& source)
	{
		std::pair<std::string, std::stringstream> returnValue;

		std::getline(source, returnValue.first, '=');
		returnValue.first.erase(std::remove(returnValue.first.begin(), returnValue.first.end(), '\t'),
			returnValue.first.end());
		returnValue.first.erase(std::remove(returnValue.first.begin(), returnValue.first.end(), ' '),
			returnValue.first.end());

		returnValue.second = std::move(getNextCodeBlock(source));

		return returnValue;
	}

	void SaveGame::processFile(const bool asPreviewOrLoad)
	{
		if (!std::ifstream::is_open()) return;

		auto& syntaxMap(asPreviewOrLoad ? loadSyntaxMap : previewSyntaxMap);
		while (!std::ifstream::eof())
		{
			std::string statement;
			std::getline(*this, statement, '=');
			statement.erase(std::remove(statement.begin(), statement.end(), '\t'), statement.end());
			statement.erase(std::remove(statement.begin(), statement.end(), ' '), statement.end());

			if (syntaxMap.count(statement))
				syntaxMap.at(statement)(getNextCodeBlock(*this));
			else skipNextCodeBlock(*this);
		}

		std::ifstream::clear();
		std::ifstream::seekg(0, std::ios::beg);
	}

	std::stringstream SaveGame::getNextCodeBlock(std::istream& source)
	{
		std::stringstream returnValue;

		skipWhitespace(source);
		if (source.eof()) return returnValue;

		char input;
		source >> input;

		if (input != '{')
		{
			while ((input != ' ' && input != '\t' && input != '\n') && !source.eof())
			{
				returnValue.putback(input);
				source >> input;
			}
			skipWhitespace(source);
			return returnValue;
		}

		std::stack<const char> brackets({ '{' });
		do
		{
			skipWhitespace(source);
			source >> input;

			if (input == '{')
				brackets.push('{');
			else if (input == '}')
				{
					brackets.pop();
					if (brackets.empty()) break;
				}

			returnValue.putback(input);
		} while (!source.eof());

		return returnValue;
	}

	void SaveGame::skipNextCodeBlock(std::istream& source)
	{
		skipWhitespace(source);
		if (source.eof()) return;

		char input;
		source >> input;

		if (input != '{')
		{
			while ((input != ' ' && input != '\t' && input != '\n') && !source.eof())
				source >> input;
			skipWhitespace(source);
			return;
		}

		std::stack<const char> brackets({ '{' });
		do
		{
			skipWhitespace(source);
			source >> input;

			if (input == '{')
				brackets.push('{');
			else if (input == '}')
			{
				brackets.pop();
				if (brackets.empty()) break;
			}
		} while (!source.eof());
	}

	void SaveGame::skipWhitespace(std::istream& source)
	{
		char input;
		do source >> input; while ((input == '\t') && !source.eof());
		if (!source.eof()) source.seekg(int(source.tellg()) - 1);
	}
}
