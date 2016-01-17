#ifndef GAME_GUI
#define GAME_GUI

#include "Nation.h"
#include "..\..\Backend\Source Files\Resources.h"
#include "Date.h"

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <GUI.h>

namespace fEnd
{
	class Topbar final : public gui::Interactive
	{
	public:
		Topbar(const Nation&);
		~Topbar()override = default;

		Topbar() = delete;
		Topbar(const Topbar&) = delete;
		Topbar(Topbar&&) = delete;
		Topbar& operator=(const Topbar&) = delete;
		Topbar& operator=(const Topbar&&) = delete;

		const sf::FloatRect getGlobalBounds()const override;
		const sf::Vector2f& getPosition()const override;
		Topbar& setPosition(const float, const float)override;

		const bool input(const sf::Event&)override;

	private:
		void updateValues(const Nation&);
		void draw(sf::RenderTarget&, sf::RenderStates)const override;

		std::unordered_map<bEnd::Resource, std::pair<gui::Icon, gui::TextArea>> resources;
		std::pair<gui::Icon, gui::TextArea>                                     manpower, diplomaticInfluence, freeSpies, officerRatio, dissent, NationalUnity;
		std::pair<gui::Icon, gui::TextPane>                                     IC;
		sf::RectangleShape                                                      dateRectangle;
		Date                                                                    currentDate;
		sf::Text                                                                nationName;
		std::vector<std::unique_ptr<gui::Icon>>                                 buttons;
		gui::Icon                                                               background;

		static const bool ASSET_LOAD_SUCCESS, loadAssets(const std::string&);

		enum imgResource
		{
			BACKGROUND, FLAG_SHADOW, BG_SHADOW, GAME_SPEED_NULL, GAME_SPEED_ARROW, PAUSE_BUTTON, RESUME_BUTTON, PLUS_BUTTON, MINUS_BUTTON,
			MENU_BUTTON, DIPLOMACY_BUTTON, INTELLIGENCE_BUTTON, POLITICS_BUTTON, PRODUCTION_BUTTON, STATS_BUTTON, TECHNOLOGY_BUTTON, ENERGY,
			METAL, RARE_MATERIALS, CRUDE_OIL, FUEL, SUPPLIES, MONEY, MANPOWER, DIPLOMATIC_INFLUENCE, FREE_SPIES, OFFICER_RATIO, DISSENT, NATIONAL_UNITY,
			IC_ICON
		};
		static std::unordered_map<imgResource, sf::Texture> textures;
	};
};

#endif