#include "Topbar.h"

#include "../Frontend.hpp"

#include <GUI/Icon.h>

namespace fEnd
{
	Topbar::Topbar(const bEnd::Tag& tag)
		: Topbar()
	{
		setTarget(tag);
	}

	Topbar::Topbar()
	{
		gui::HoverMessage default(gui::bind("", sf::Color()), Resources::font("arial"), 13);
		default.setBackgroundFill(sf::Color(30, 30, 35, 240))
			.setBorderThickness(2)
			.setBorderFill(sf::Color(45, 45, 50, 245));
		const auto fullstop(gui::bind(".", sf::Color::White));
		const sf::Color color(231, 194, 18, 255);

		setBackgroundTexture(Resources::texture("topbar_shadow"), false);
		add("flag", gui::Icon());
		add("energy_icon", gui::Icon(Resources::texture("icon_energy"), true).setPosition(99, -1)
			.setMessage(default.setText(gui::bind("Energy\n", color) + gui::bind("Production covers various sources of power, mainly\ncoal. ", sf::Color::White)
				+ gui::bind("Energy ", color) + gui::bind("is required for ", sf::Color::White) + gui::bind("Industrial Capacity", color)
				+ fullstop)));
		add("metal_icon", gui::Icon(Resources::texture("icon_metal"), true).setPosition(165, -1)
			.setMessage(default.setText(gui::bind("Metal\n", color)
				+ gui::bind("Heavy industry requires various metal ores, primarily\niron. They are used to produce ", sf::Color::White)
				+ gui::bind("Industrial Capacity", color) + fullstop)));
		add("rare_mats_icon", gui::Icon(Resources::texture("icon_raremat"), true).setPosition(231, -1)
			.setMessage(default.setText(gui::bind("Rare Materials\n", color) + gui::bind("This covers a spectrum of materials essential \
for production,\nincluding natural products like rubber and less-common metals\nsuch as chrome and uranium.", sf::Color::White) +
				gui::bind(" Necessary for ", sf::Color::White) + gui::bind("Industrial Capacity", color) + fullstop)));
		add("oil_icon", gui::Icon(Resources::texture("icon_oil"), true).setPosition(293, -2)
			.setMessage(default.setText(gui::bind("Crude Oil\n", color) + gui::bind("Procured from drilling in oil-rich provinces or through trade and\n\
essential for any modern army to operate. Must first be refined into ", sf::Color::White) + gui::bind("Fuel", color) + fullstop)));
		add("ic_icon", gui::Icon(Resources::texture("icon_ic"), true).setPosition(363, 4)
			.setMessage(default.setText(gui::bind("Industrial Capacity\n", color) + gui::bind("This represents the combined production \
capacity of our industries.\nTo produce one point of ", sf::Color::White) + gui::bind("Industrial Capacity ", color) +
				gui::bind("we need one level of\n", sf::Color::White) + gui::bind("Industry ", color) + gui::bind("fully built, as well as 2 ", sf::Color::White) +
				gui::bind("Energy", color) + gui::bind(", 1 ", sf::Color::White) + gui::bind("Metal", color) + gui::bind(" and 0.5", sf::Color::White) +
				gui::bind(" Rare Materials", color) + fullstop)));
		add("supplies_icon", gui::Icon(Resources::texture("icon_supplies"), true).setPosition(481, -1)
			.setMessage(default.setText(gui::bind("Supplies\n", color) + gui::bind("Covers everything a modern army needs to operate\nexcept ", sf::Color::White)
				+ gui::bind("Fuel", color) + gui::bind(" - food rations, ammunition and weapons.\nThese are produced using ", sf::Color::White) + 
				gui::bind("Industrial Capacity", color))));
		

		shadow.setTexture(Resources::texture("topbar_shadow"));
		flagShadow.setTexture(Resources::texture("topbarflag_shadow"));
	}

	Topbar& Topbar::setTarget(const bEnd::Tag& tag)
	{
		target = tag;

		setBackgroundTexture(Resources::texture(Resources::textureExists("topbar_" + std::string(tag)) ? "topbar_" + std::string(tag) : "topbar_generic"));
		((gui::Icon&)at("flag")).setTexture(Nation::get(tag).getFlag());

		return *this;
	}

	std::unique_ptr<gui::Window> Topbar::copy() const
	{
		return std::unique_ptr<gui::Window>(new Topbar(*this));
	}

	std::unique_ptr<gui::Window> Topbar::move()
	{
		return std::unique_ptr<gui::Window>(new Topbar(std::move(*this)));
	}

	void Topbar::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(shadow, states);
		Window::draw(target, states);
		target.draw(flagShadow);
	}
}