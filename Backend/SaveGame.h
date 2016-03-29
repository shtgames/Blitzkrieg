#ifndef SAVEGAME_BACKEND
#define SAVEGAME_BACKEND

#include <sstream>
#include <fstream>
#include <unordered_map>
#include <functional>
#include <memory>

namespace bEnd
{
	class SaveGame final : private std::ifstream
	{
	public:
		struct Statement final
		{
			friend class SaveGame;
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

		SaveGame(const SaveGame& copy) = default;
		SaveGame(SaveGame&& temp) = default;
		SaveGame() = default;
		~SaveGame() = default;

		SaveGame& operator=(const SaveGame& copy) = default;
		SaveGame& operator=(SaveGame&& temp) = default;

		const bool open(const std::string& filePath);

		void preview();
		void load();

		static void addSyntax(const std::string& statement, const std::function<void(const Statement&)>& codeBlockProcessor, const bool validInPreviewOrLoad = 1);
		static void addSyntax(const std::string& statement, std::function<void(const Statement&)>&& codeBlockProcessor, const bool validInPreviewOrLoad = 1);

	private:
		void processFile(const bool asPreviewOrLoad);

		std::vector<Statement> statements;
		bool statementsVectorNeedsUpdate = true;

		static const Statement getNextStatement(std::istream& source);
		static void skipWhitespace(std::istream& source);

		static std::unordered_map<std::string, std::function<void(const Statement&)>> previewSyntaxMap, loadSyntaxMap;
	};
}

#endif