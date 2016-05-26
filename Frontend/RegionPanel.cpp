#include "RegionPanel.h"

#include "../Backend/Unit.h"
#include "../Backend/Region.h"
#include "../Frontend.hpp"

#include <GUI/Button.h>
#include <GUI/ProgressBar.h>

namespace fEnd
{
	RegionPanel::RegionPanel(const sf::Vector2u& resolution)
	{
		setBackgroundTexture(Resources::texture("bg_province"), true);

		gui::ProgressBar bar1(gui::Icon(Resources::texture("damaged_building_levels"), true), gui::Icon(Resources::texture("damaged_building_levels"), true)),
			 bar2(gui::Icon(Resources::texture("building_levels"), true), gui::Icon(Resources::texture("building_levels"), true));
		const auto func([](const std::string& key, const bool second) -> float
		{
			if (Map::target)
				return int(second ? bEnd::Region::get(*Map::target).getBuildingLevels(key).second : bEnd::Region::get(*Map::target).getBuildingLevels(key).first) / 10.0f;
			else return 0;
		});

		sf::Vector2f translate(0, 0);
		const auto size(Resources::texture("bg_province").getSize());

		for (auto it(bEnd::Unit::begin()), end(bEnd::Unit::end()); it != end; ++it)
			if (it->second.getType() != bEnd::Unit::Building) continue;
			else
			{
				add(it->first + "_btn", gui::Button(gui::Icon(Resources::texture("prov_build_" + it->first), true))
					.setPosition(35 + translate.x - Resources::texture("prov_build_" + it->first).getSize().x, size.y - 61 + translate.y +
						(Resources::texture("building_levels").getSize().y - Resources::texture("prov_build_" + it->first).getSize().y) / 2));
				add(it->first + "_dmgbar", bar1.setUpdateFunction(std::bind(func, it->first, 1)).setPosition(39 + translate.x, size.y - 61 + translate.y));
				add(it->first + "_bar", bar2.setUpdateFunction(std::bind(func, it->first, 0)).setPosition(39 + translate.x, size.y - 61 + translate.y));

				translate.y -= Resources::texture("building_levels").getSize().y + 8;
				if (count() / 3 == bEnd::Unit::unitsOfType(bEnd::Unit::Building) / 2)
				{
					translate.x += Resources::texture("building_levels").getSize().x + 48;
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
