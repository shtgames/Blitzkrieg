#include "FileProcessor.h"
#include "FileProcessor.h"

#include "Unit.h"
#include "Tech.h"
#include "TimeSystem.h"
#include "Nation.h"
#include "Region.h"

#include <algorithm>
#include <stack>
#include <cctype>
#include <io.h>

namespace bEnd 
{
	const long long FileProcessor::unlimitedStreamsize(std::numeric_limits<std::streamsize>::max());

	void load()
	{
		auto source(std::move(getDirectoryContents("units/*.*")));

		for (auto it = source.begin(), end = source.end(); it != end; ++it)
		{
			FileProcessor file(*it);

			for (auto it1 = file.getStatements().begin(), end1 = file.getStatements().end(); it1 != end1; ++it1)
				Unit::loadFromFile(*it1);
		}
		
		source = std::move(getDirectoryContents("technologies/*.*"));

		for (auto it = source.begin(), end = source.end(); it != end; ++it)
		{
			FileProcessor file(*it);
			for (auto it1 = file.getStatements().begin(), end1 = file.getStatements().end(); it1 != end1; ++it1)
				Tech::loadFromFile(*it1);
		}
	}

	const Date readDate(std::string source)
	{
		Date returnValue;
		std::stringstream ss(source);
		
		ss.ignore(FileProcessor::unlimitedStreamsize, '"');
		std::getline(ss, source, '.');
		returnValue.setYear(std::stoi(source.c_str()));
		std::getline(ss, source, '.');
		returnValue.setMonth(std::stoi(source.c_str()));
		std::getline(ss, source, '.');
		returnValue.setDay(std::stoi(source.c_str()));
		std::getline(ss, source, '.');
		returnValue.setHour(std::stoi(source.c_str()));

		return returnValue;
	}

	void loadSavedGame(const FileProcessor& source)
	{
		if (!source.isOpen()) return;
		
		for (auto it = source.getStatements().begin(), end = source.getStatements().end(); it != end; ++it)
			if (it->lValue == "date") TimeSystem::reset(readDate(it->rStrings.front()));
			else if (it->lValue == "player")
			{
				std::stringstream ss(it->rStrings.front());
				ss.ignore(FileProcessor::unlimitedStreamsize, '"');
				std::string buffer;
				std::getline(ss, buffer, '"');
				Nation::player = buffer;
			}
			else if (!it->lValue.empty() && std::find_if(it->lValue.begin(),
				it->lValue.end(), [](char c) { return !std::isdigit(c); }) == it->lValue.end())
				Region::loadFromSave(*it);
			else if (Tag::isTag(it->lValue))
				Nation::loadFromSave(*it);
	}

	const std::vector<std::string> getDirectoryContents(const std::string& path)
	{
		_finddata_t target;
		std::vector<std::string> returnValue;

		auto start = _findfirst(path.c_str(), &target);
		if (start == -1) return returnValue;

		returnValue.emplace_back();
		returnValue.back().assign(target.name);

		while (true)
		{
			auto next = _findnext(start, &target);
			if (next == -1) break;
			if (target.attrib & _A_SUBDIR != _A_SUBDIR)
			{
				returnValue.emplace_back();
				returnValue.back().assign(target.name);
			}
		}

		_findclose(start);

		return returnValue;
	}

	FileProcessor::FileProcessor(const std::string& path)
		: FileProcessor()
	{
		open(path);
	}

	const bool FileProcessor::isOpen() const
	{
		return std::ifstream::is_open();
	}

	const bool FileProcessor::open(const std::string& filePath)
	{
		std::ifstream::open(filePath);
		if (std::ifstream::is_open())
		{
			while (!std::ifstream::eof())
				statements.emplace_back(std::move(getNextStatement(*this)));
			std::ifstream::clear();
			std::ifstream::seekg(0, std::ios::beg);

			return true;
		}
		statements.clear();
		return false;
	}

	void FileProcessor::close()
	{
		std::ifstream::close();
	}

	const std::vector<FileProcessor::Statement>& FileProcessor::getStatements() const
	{
		return statements;
	}

	const FileProcessor::Statement FileProcessor::getNextStatement(std::istream& source)
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

	void FileProcessor::skipWhitespace(std::istream& source)
	{
		char input;
		do 
		{
			source.get(input);
			if (input != ' ' && input != '\n' && input != '\t')
			{
				if (input == '#') source.ignore(unlimitedStreamsize, '\n');
				else break;
			}
		} while (!source.eof());
		if (!source.eof()) source.seekg(int(source.tellg()) - 1);
	}
}
