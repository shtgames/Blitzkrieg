#include "FileProcessor.h"

#include "Unit.h"
#include "Tech.h"
#include "TimeSystem.h"
#include "Nation.h"
#include "Province.h"

#include <algorithm>
#include <stack>
#include <cctype>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <regex>

namespace bEnd 
{
	std::string currentSave;
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

	const std::string& currentlyLoadedFile()
	{
		return currentSave;
	}

	const Date readDate(std::string source)
	{
		Date returnValue;
		std::stringstream ss(source);
		
		std::getline(ss, source, '.');
		returnValue.setYear(std::stoi(source));
		std::getline(ss, source, '.');
		returnValue.setMonth(std::stoi(source));
		std::getline(ss, source, '.');
		returnValue.setDay(std::stoi(source));
		std::getline(ss, source, '.');
		returnValue.setHour(std::stoi(source));

		return returnValue;
	}

	void loadSavedGame(const std::string& path)
	{
		FileProcessor source("save game/" + path);
		if (!source.isOpen()) return;

		currentSave.clear();
		for (size_t i(0), end(path.size()); i != end && path[i] != '.'; ++i)
			currentSave.push_back(path[i]);

		Nation::reset();

		for (auto& it : source.getStatements())
			if (it.lValue == "date") TimeSystem::reset(readDate(it.rStrings.front()));
			else if (it.lValue == "player")
				Nation::player = it.rStrings.front();
			else if (!it.lValue.empty() && std::find_if(it.lValue.begin(),
					it.lValue.end(), [](const char c) { return !std::isdigit(c); }) == it.lValue.end())
				Province::loadFromSave(it);
			else if (Tag::isTag(it.lValue))
				Nation::loadFromSave(it);
	}

	const std::vector<std::string> getDirectoryContents(const std::string& path, const std::string& filenameRegex)
	{
		std::vector<std::string> returnValue;

		if (!boost::filesystem::exists(path) || !boost::filesystem::is_directory(path)) return returnValue;

		const std::regex filename(filenameRegex);

		for (const auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(path), {}))
			if (std::regex_match(entry.path().filename().string() + entry.path().extension().string(), filename)
				&& boost::filesystem::is_regular_file(entry.path().string()))
				returnValue.push_back(entry.path().filename().string() + entry.path().extension().string());

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
			while (int(std::ifstream::tellg()) != -1)
				statements.emplace_back(std::move(*getNextStatement(*this)));
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

	const std::unique_ptr<FileProcessor::Statement> FileProcessor::getNextStatement(std::istream& source)
	{
		std::unique_ptr<Statement> returnValue(new Statement());

		returnValue->lValue = std::move(getLeftHandSide(source));
		auto nextChar(source.get());
		std::string buffer;

		if (nextChar == '{')
			while (!source.eof())
			{
				skipWhitespace(source);

				nextChar = source.get();
				if (nextChar == '}' || nextChar == std::char_traits<char>::eof())
				{
					nextChar == '}' ? skipWhitespace(source) : source.setstate(std::istream::eofbit);
					break;
				}
				source.unget();

				const auto pos(source.tellg());
				std::getline(source, buffer, '\n');
				
				if (buffer.find_first_of('=') != std::string::npos)
				{
					source.seekg(pos);
					buffer.clear();
					returnValue->rStatements.emplace_back(std::move(*getNextStatement(source)));
				}
				else
				{
					source.seekg(pos);
					if (!buffer.empty() && buffer.front() == '"')
					{
						source.ignore(unlimitedStreamsize, '"');
						std::getline(source, buffer, '"');
					}
					else source >> buffer;
					returnValue->rStrings.emplace_back(std::move(buffer));
					if (!returnValue->rStrings.back().empty() && returnValue->rStrings.back().back() == '}')
					{
						returnValue->rStrings.back().pop_back();
						break;
					}
				}
			}
		else if (nextChar == '"')
		{
			std::getline(source, buffer, '"');
			skipWhitespace(source);
			returnValue->rStrings.emplace_back(std::move(buffer));
		}
		else
		{
			source.unget();
			source >> buffer;
			skipWhitespace(source);
			returnValue->rStrings.emplace_back(std::move(buffer));
		}

		return returnValue;
	}

	const std::string FileProcessor::getLeftHandSide(std::istream& source)
	{
		std::string returnValue;

		skipWhitespace(source);
		std::getline(source, returnValue, '=');
		skipWhitespace(source);

		returnValue.erase(std::remove_if(returnValue.begin(), returnValue.end(), [&](const char element)
			{
				return element == ' ' || element == '\t' || element == '\n';
			}),
			returnValue.end());

		return returnValue;
	}
	
	void FileProcessor::skipWhitespace(std::istream& source)
	{
		do 
		{
			const auto input = source.get();
			if (input != ' ' && input != '\n' && input != '\t')
			{
				if (input == '#') source.ignore(unlimitedStreamsize, '\n');
				else break;
			}
			else if (input == -1)
			{
				source.setstate(std::ios_base::eofbit);
				break;
			}
		} while (!source.eof());
		if (!source.eof()) source.unget();
	}
}
