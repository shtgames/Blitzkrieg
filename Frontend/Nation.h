#ifndef NATION_FRONTEND
#define NATION_FRONTEND

#include <SFML/Graphics.hpp>

#include <memory>
#include <unordered_map>
#include <mutex>

#include <iostream>

#include "../Backend/Tag.h"

namespace fEnd
{
	class Nation final
	{
	public:
		enum class Continent
		{
			Europe,
			Asia,
			Africa,
			Oceania,
			NorthAmerica,
			SouthAmerica
		};

		Nation(const Nation&) = default;
		Nation(Nation&&) = default;
		Nation() = default;
		~Nation() = default;

		void loadFromFile(const std::string& path);

		const sf::Texture& getFlag()const { return flag; }
		const sf::Color& getColor()const { return color; }
		const std::string& getName()const { return name; }

		static Nation& get(const bEnd::Tag& tag) { std::lock_guard<std::mutex> guard(nationsLock); return nations[tag]; }
		static void loadNations();

	private:
		sf::Texture flag;
		sf::Color color = sf::Color(180, 180, 180);
		std::string name;
		Continent continent;

		static std::mutex nationsLock;
		static std::unordered_map<bEnd::Tag, Nation> nations;
	};
}

#endif