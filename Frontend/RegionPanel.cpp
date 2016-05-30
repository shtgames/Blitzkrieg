#include "RegionPanel.h"

#include "../Backend/Unit.h"
#include "../Backend/Region.h"
#include "../Backend/Production.h"
#include "../Backend/Nation.h"
#include "../Frontend.hpp"

#include <GUI/Button.h>
#include <GUI/ProgressBar.h>

namespace fEnd
{
	const auto updateFunction([](const std::string& key, const bool second) -> const float
	{
		if (Map::target)
			return unsigned char(second ? bEnd::Region::get(*Map::target).getBuildingLevels(key).second : bEnd::Region::get(*Map::target).getBuildingLevels(key).first) / 10.0f;
		else return 0;
	});

	class BuildingLevels final : public gui::Interactive
	{
	public:
		BuildingLevels(const BuildingLevels& copy) = default;
		BuildingLevels(BuildingLevels&& temp) = default;
		BuildingLevels(const std::string& building)
			: queued(gui::Icon(Resources::texture("building_bar_bg")), gui::Icon(Resources::texture("queued_building_count"))),
			damaged(gui::Icon(Resources::texture("building_bar_bg")), gui::Icon(Resources::texture("damaged_building_levels"))),
			built(gui::Icon(Resources::texture("building_bar_bg")), gui::Icon(Resources::texture("building_levels"))),
			build(gui::Icon(Resources::texture("prov_build_" + building), true))
		{
			queued.setUpdateFunction([building]() -> float
			{
				if (Map::target)
					return (bEnd::Region::get(*Map::target).getBuildingLevels(building).second + bEnd::Region::get(*Map::target).getQueuedCount(building)) / 10.0f;
				else return 0;
			})
				.setFillMessage(gui::HoverMessage(gui::bind("") + [building]()
					{
						if (!Map::target) return gui::bind("");
						const auto levels(bEnd::Region::get(*Map::target).getBuildingLevels(building));
						return levels.first < levels.second ? gui::bind("This building has been damaged and is currently at ") + gui::bind(std::to_string(levels.first), sf::Color::Yellow) +
							gui::bind(".\nIt is built up to level ") + gui::bind(std::to_string(levels.second), sf::Color::Yellow) + gui::bind(" but will take time to repair.\nFurther combat may damage it even more.")
							: (levels.second ? gui::bind("This building is built up to level ") + gui::bind(std::to_string(levels.second), sf::Color::Yellow) + gui::bind(".") : gui::bind("")) +
							(bEnd::Region::get(*Map::target).getQueuedCount(building) ? gui::bind(" Currently building more.") : gui::bind(""));
					}, Resources::font("arial"), 13));
			damaged.setUpdateFunction(std::bind(updateFunction, building, 1));
			built.setUpdateFunction(std::bind(updateFunction, building, 0));
			build.bindAction(gui::Released, [building]()
			{
				if (!Map::target) return;
				bEnd::Production::get(bEnd::Nation::player).addProductionItem(building, *Map::target);
			})
				.setPredicates(gui::Button::PredicateArray{ [building]()
			{
				return Map::target && bEnd::Region::get(*Map::target).getController() == bEnd::Nation::player &&
					bEnd::Region::get(*Map::target).getBuildingLevels(building).second + bEnd::Region::get(*Map::target).getQueuedCount(building) < 10;
			} })
				.setMessage(gui::HoverMessage(gui::bind(building), Resources::font("arial"), 13));
		}

		std::unique_ptr<gui::Interactive> copy()const override
		{
			return std::unique_ptr<gui::Interactive>(new auto(*this));
		}
		std::unique_ptr<gui::Interactive> move()override
		{
			return std::unique_ptr<gui::Interactive>(new auto(std::move(*this)));
		}

		const sf::FloatRect getGlobalBounds()const override
		{
			return damaged.getGlobalBounds();
		}
		const sf::Vector2f& getPosition()const override
		{
			return damaged.getPosition();
		}
		BuildingLevels& setPosition(const float x, const float y)
		{
			queued.setPosition(x, y);
			damaged.setPosition(x, y);
			built.setPosition(x, y);
			build.setPosition(x - (4 + build.getGlobalBounds().width), y + (built.getGlobalBounds().height - build.getGlobalBounds().height) / 2);
			return *this;
		}

		const bool input(const sf::Event& event)
		{
			return build.input(event) || queued.input(event) || built.input(event) || damaged.input(event);
		}

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override
		{
			target.draw(queued, states);
			target.draw(damaged, states);
			target.draw(built, states);
			target.draw(build, states);
		}

		gui::Button build;
		gui::ProgressBar queued, damaged, built;
	};

	RegionPanel::RegionPanel(const sf::Vector2u& resolution)
	{
		setBackgroundTexture(Resources::texture("bg_province"), true);

		const auto updateFunction([](const std::string& key, const bool second) -> const float
		{
			if (Map::target)
				return unsigned char(second ? bEnd::Region::get(*Map::target).getBuildingLevels(key).second : bEnd::Region::get(*Map::target).getBuildingLevels(key).first) / 10.0f;
			else return 0;
		});

		sf::Vector2f translate(0, 0);
		const auto size(Resources::texture("bg_province").getSize());

		for (auto it(bEnd::Unit::begin()), end(bEnd::Unit::end()); it != end; ++it)
			if (it->second.getType() != bEnd::Unit::Building) continue;
			else
			{
				add(it->first, BuildingLevels(it->first).setPosition(39 + translate.x, size.y - 61 + translate.y));

				translate.y -= Resources::texture("building_levels").getSize().y + 8;
				if (count() == bEnd::Unit::unitsOfType(bEnd::Unit::Building) / 2)
				{
					translate.x += Resources::texture("building_levels").getSize().x + 49;
					translate.y = 0;
				}
			}

		setPosition(0, resolution.y - size.y);
	}

	std::unique_ptr<gui::Window> RegionPanel::copy() const
	{
		return std::unique_ptr<gui::Window>(new RegionPanel(*this));
	}

	std::unique_ptr<gui::Window> RegionPanel::move()
	{
		return std::unique_ptr<gui::Window>(new RegionPanel(std::move(*this)));
	}

	const bool RegionPanel::input(const sf::Event& event)
	{
		if (!Map::target || bEnd::Region::get(*Map::target).isSea()) return false;
		return Window::input(event);
	}

	void RegionPanel::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		if (!Map::target || bEnd::Region::get(*Map::target).isSea()) return;
		Window::draw(target, states);
	}
}
