#ifndef SAVEGAME_BACKEND
#define SAVEGAME_BACKEND

#include <sstream>
#include <fstream>
#include <unordered_map>
#include <functional>
#include <memory>

namespace bEnd
{
	class FileProcessor;

	void load();
	void loadSavedGame(const FileProcessor& source);
	const std::vector<std::string> getDirectoryContents(const std::string& path);

	class FileProcessor final : private std::ifstream
	{
	public:
		struct Statement final
		{
			friend class FileProcessor;
		public:
			Statement(const Statement& copy) = default;
			Statement(Statement&& temp) = default;
			~Statement() = default;

			Statement& operator=(const Statement& copy) = default;
			Statement& operator=(Statement&& temp) = default;

			std::string lValue;
			std::vector<std::string> rStrings;
			std::vector<Statement> rStatements;

		private:
			Statement() = default;
		};

		FileProcessor(const FileProcessor& copy) = default;
		FileProcessor(FileProcessor&& temp) = default;
		FileProcessor() = default;
		~FileProcessor() = default;
		FileProcessor(const std::string& path);

		FileProcessor& operator=(const FileProcessor& copy) = default;
		FileProcessor& operator=(FileProcessor&& temp) = default;

		const bool isOpen()const;
		const bool open(const std::string& filePath);
		void close();

		const std::vector<Statement>& getStatements()const;

		static const long long unlimitedStreamsize;

	private:
		std::vector<Statement> statements;

		static const Statement getNextStatement(std::istream& source);
		static void skipWhitespace(std::istream& source);
	};
}

#endif