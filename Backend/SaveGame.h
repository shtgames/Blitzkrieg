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
		const bool open(const std::string& filePath);

		void preview();
		void load();

		static void addSyntax(const std::string& statement, const std::function<void(const std::stringstream&)>& codeBlockProcessor, const bool validInPreviewOrLoad = 1);
		static void addSyntax(const std::string& statement, std::function<void(const std::stringstream&)>&& codeBlockProcessor, const bool validInPreviewOrLoad = 1);
		static std::pair<std::string, std::stringstream> getNextStatement(std::istream& source);

	private:
		void processFile(const bool asPreviewOrLoad);

		static std::stringstream getNextCodeBlock(std::istream& source);
		static void skipNextCodeBlock(std::istream& source);
		static void skipWhitespace(std::istream& source);

		static std::unordered_map<std::string, std::function<void(const std::stringstream&)>> previewSyntaxMap, loadSyntaxMap;
	};
}

#endif