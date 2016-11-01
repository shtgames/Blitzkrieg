#include "ProvincePanel.h"

#include "../Backend/Unit.h"
#include "../Backend/Province.h"
#include "../Backend/Production.h"
#include "../Backend/Nation.h"
#include "../Frontend.hpp"

#include <GUI/Button.h>
#include <GUI/ProgressBar.h>

namespace fEnd
{
	const std::string textureTransitionShader =
		"uniform sampler2D tex1, tex2, shadow;\
		uniform float state;\
		uniform bool active;\
		\
		void main()\
		{\
			vec4 color = mix(mix(texture2D(tex1, gl_TexCoord[0].xy), texture2D(tex2, gl_TexCoord[0].xy), gl_TexCoord[0].x), texture2D(shadow, gl_TexCoord[0].xy), texture2D(shadow, gl_TexCoord[0].xy).a) * gl_Color;\
			if (active)\
				gl_FragColor = vec4(color.rgb * (1.0f - (state * 0.15f)), color.a);\
			else\
			{\
				float greyValue = color.r * 0.29 + color.g * 0.58 + color.b * 0.13;\
				gl_FragColor = vec4(greyValue, greyValue, greyValue, color.a);\
			}\
		}";

	const auto updateFunction([](const std::string& key, const bool second) -> const float
	{
		if (Map::target)
			return (unsigned char)(second ? bEnd::Province::get(*Map::target).getBuildingLevels(key).second :
					bEnd::Province::get(*Map::target).getBuildingLevels(key).first) / 10.0f;
		else return 0;
	});

	gui::HoverMessage defaultTooltip(gui::HoverMessage()
		.setCharacterSize(13)
		.setBackgroundFill(sf::Color(30, 30, 35, 240))
		.setBorderThickness(2)
		.setBorderFill(sf::Color(45, 45, 50, 245)));
	const sf::Color color(231, 194, 18);

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
							return (bEnd::Province::get(*Map::target).getBuildingLevels(building).second + bEnd::Province::get(*Map::target).getQueuedCount(building)) / 10.0f;
						else return 0;
					})
				.setFillMessage(defaultTooltip.setText(gui::bind("") + [building]()
					{
						if (!Map::target) return gui::bind("");
						const auto levels(bEnd::Province::get(*Map::target).getBuildingLevels(building));
						return levels.first < levels.second ? gui::bind("This building has been damaged and is currently at ") + gui::bind(std::to_string(levels.first), color) +
							gui::bind(".\nIt is built up to level ") + gui::bind(std::to_string(levels.second), color) + gui::bind(" but will take time to repair.\nFurther combat may damage it even more.")
							: (levels.second ? gui::bind("This building is built up to level ") + gui::bind(std::to_string(levels.second), color) + gui::bind(".") : gui::bind("")) +
							(bEnd::Province::get(*Map::target).getQueuedCount(building) ? gui::bind(" Currently building more.") : gui::bind(""));
					}));
			damaged.setUpdateFunction(std::bind(updateFunction, building, 1));
			built.setUpdateFunction(std::bind(updateFunction, building, 0));
			build.bindAction(gui::Released, [building]()
					{
						if (!Map::target) return;
						bEnd::Production::get(bEnd::Nation::player).addProductionItem(building, *Map::target);
					})
				.setPredicates(gui::Button::PredicateArray{ [building]()
					{
						return Map::target && bEnd::Province::get(*Map::target).getController() == bEnd::Nation::player &&
							bEnd::Province::get(*Map::target).getBuildingLevels(building).second + bEnd::Province::get(*Map::target).getQueuedCount(building) < 10 &&
							!(bEnd::Unit::get(building).isCoastal() && !bEnd::Province::get(*Map::target).isCoastal());
					} })
				.setMessage(defaultTooltip.setText(gui::bind(building)));
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
		void setParent(const gui::Window* const parent)override
		{
			Interactive::setParent(parent);
			build.setParentToSame(*this);
			queued.setParentToSame(*this);
			damaged.setParentToSame(*this);
			built.setParentToSame(*this);
		}

		gui::Button build;
		gui::ProgressBar queued, damaged, built;
	};

	ProvincePanel::ProvincePanel(const sf::Vector2u& resolution)
		: Window()
	{
		defaultTooltip.setFont(Resources::font("arial"));
		setBackgroundTexture(Resources::texture("bg_province"), true);
		const auto size(Resources::texture("bg_province").getSize());

		{
			const auto updateFunction([](const std::string& key, const bool second) -> const float
			{
				if (Map::target)
					return (unsigned char)(second ? bEnd::Province::get(*Map::target).getBuildingLevels(key).second :
							bEnd::Province::get(*Map::target).getBuildingLevels(key).first) / 10.0f;
				else return 0;
			});

			sf::Vector2f translate(0, 0);

			for (auto it(bEnd::Unit::begin()), end(bEnd::Unit::end()); it != end; ++it)
				if (bEnd::Unit::get(*it).getType() != bEnd::Unit::Building) continue;
				else
				{
					add(*it, BuildingLevels(*it).setPosition(39 + translate.x, size.y - 61 + translate.y));

					translate.y -= Resources::texture("building_levels").getSize().y + 8;
					if (count() == bEnd::Unit::unitsOfType(bEnd::Unit::Building) / 2)
					{
						translate.x += Resources::texture("building_levels").getSize().x + 49;
						translate.y = 0;
					}
				}
		}

		add("owner_flag", gui::Button(gui::Icon(Resources::texture("topbarflag_shadow"))).setPosition(10 + Resources::texture("topbarflag_shadow").getSize().y, 39)
				.resetShader(textureTransitionShader).setShaderParameter("shadow", sf::Shader::CurrentTexture)
				.setRotation(90)
				.setMessage(defaultTooltip.setText(gui::bind("This province is owned by ") + []()
					{
						if (!Map::target) return gui::bind("N/A");
						return gui::bind(Nation::get(bEnd::Province::get(*Map::target).getOwner()).getName(), color) +
							(bEnd::Province::get(*Map::target).getOwner() == bEnd::Province::get(*Map::target).getController() ? gui::bind("") :
								gui::bind(" but has been occupied by ") + gui::bind(Nation::get(bEnd::Province::get(*Map::target).getController()).getName(), color));
					} + gui::bind("."))))
		.add("name", gui::TextArea().setPosition(6, 11).setFont(Resources::font("arial")).setCharacterSize(13).setUpdateFunction([]()
			{
				if (!Map::target) return gui::bind("No Province Selected");
				return gui::bind(Map::get(*Map::target).name, sf::Color(203, 200, 201));
			}))
		.add("contested", gui::TextArea().setPosition(180, 160).setFont(Resources::font("arial")).setCharacterSize(13).setStyle(sf::Text::Bold | sf::Text::Italic).setUpdateFunction([]()
			{
				if (!Map::target) return gui::bind("");
				return bEnd::Province::get(*Map::target).getController() != bEnd::Province::get(*Map::target).getOwner() ? gui::bind("Contested", sf::Color(191, 80, 73)) : gui::bind("");
			}))
		.add("neighbours", gui::TextArea("", Resources::font("arial"), 13).setPosition(82, 37)
			.setUpdateFunction([]()
				{
					if (!Map::target) return gui::bind("Neighbours: N/A");
					sf::String returnValue("Neighbours:\n");
					for (const auto& it : bEnd::Province::get(*Map::target).getNeighbours())
						returnValue += Map::get(it.first).name + ": " + std::to_string(it.second) + " km\n";
					return gui::bind(returnValue, sf::Color(150, 150, 150));
				}));

		setPosition(0, resolution.y - size.y);
	}

	std::unique_ptr<gui::Window> ProvincePanel::copy() const
	{
		return std::unique_ptr<gui::Window>(new ProvincePanel(*this));
	}

	std::unique_ptr<gui::Window> ProvincePanel::move()
	{
		return std::unique_ptr<gui::Window>(new ProvincePanel(std::move(*this)));
	}

	const bool ProvincePanel::input(const sf::Event& event)
	{
		if (!Map::target || bEnd::Province::get(*Map::target).isSea()) return false;
		if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape)
		{
			Map::deselect();
			return true;
		}
		return Window::input(event);
	}

	void ProvincePanel::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		if (!Map::target || bEnd::Province::get(*Map::target).isSea()) return;
		((gui::Button&)at("owner_flag")).setShaderParameter("tex1", Nation::get(bEnd::Province::get(*Map::target).getOwner()).getFlag())
			.setShaderParameter("tex2", Nation::get(bEnd::Province::get(*Map::target).getController()).getFlag());
		Window::draw(target, states);
	}

	void ProvincePanel::setParent(const gui::WindowManager* const parent) {}
}
