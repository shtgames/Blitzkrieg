#include "Topbar.h"

#include "Date.h"
#include "../Frontend.hpp"
#include "../Backend/TimeSystem.h"
#include "../Backend/ResourceDistributor.h"
#include "../Backend/LeadershipDistributor.h"
#include "../Backend/Nation.h"

#include <GUI/Icon.h>
#include <GUI/TextArea.h>
#include <GUI/CheckBox.h>
#include <GUI/AudioSystem.h>

#include <iomanip>

namespace fEnd
{
	Topbar::Topbar(const bEnd::Tag& tag)
		: Topbar()
	{
		setTarget(tag);
	}

	Topbar::Topbar()
	{
		const auto& font(Resources::font("arial"));
		gui::HoverMessage defaultTooltip(gui::bind("", sf::Color()), font, 13);
		defaultTooltip.setBackgroundFill(sf::Color(30, 30, 35, 240))
			.setBorderThickness(2)
			.setBorderFill(sf::Color(45, 45, 50, 245));
		const auto fullstop(gui::bind("."));
		const sf::Color color(231, 194, 18);

		add("flag", gui::Icon());
		add("energy_icon", gui::Icon(Resources::texture("icon_energy"), true).setPosition(95, -1)
			.setMessage(defaultTooltip.setText(gui::bind("Energy\n", color) + gui::bind("Production covers various sources of power, mainly\ncoal. ")
				+ gui::bind("Energy ", color) + gui::bind("is required for ") + gui::bind("Industrial Capacity", color)
				+ fullstop)));
		add("metal_icon", gui::Icon(Resources::texture("icon_metal"), true).setPosition(163, 0)
			.setMessage(defaultTooltip.setText(gui::bind("Metal\n", color)
				+ gui::bind("Heavy industry requires various metal ores, primarily\niron. They are used to produce ")
				+ gui::bind("Industrial Capacity", color) + fullstop)));
		add("rare_mats_icon", gui::Icon(Resources::texture("icon_raremat"), true).setPosition(228, 0)
			.setMessage(defaultTooltip.setText(gui::bind("Rare Materials\n", color) + gui::bind("This covers a spectrum of materials essential \
for production, including\nnatural products like rubber and less-common metals such as chrome\nand uranium.") +
				gui::bind(" Necessary for ") + gui::bind("Industrial Capacity", color) + fullstop)));
		add("oil_icon", gui::Icon(Resources::texture("icon_oil"), true).setPosition(288, -2)
			.setMessage(defaultTooltip.setText(gui::bind("Crude Oil\n", color) + gui::bind("Procured from drilling in oil-rich Provinces or through trade and \
essential\nfor any modern army to operate. Must first be refined into ") + gui::bind("Fuel", color) + fullstop)));
		add("ic_icon", gui::Icon(Resources::texture("icon_ic"), true).setPosition(362, 4)
			.setMessage(defaultTooltip.setText(gui::bind("Industrial Capacity\n", color) + gui::bind("This represents the combined production \
capacity of our industries. To\nproduce one point of ") + gui::bind("Industrial Capacity ", color) +
				gui::bind("we need one level of ") + gui::bind("Industry\n", color) + gui::bind("fully built, as well as 2 ") +
				gui::bind("Energy", color) + gui::bind(", 1 ") + gui::bind("Metal", color) + gui::bind(" and 0.5") +
				gui::bind(" Rare Materials", color) + fullstop + gui::bind("\nThe first number is the amount of ") + gui::bind("IC", color) +
				gui::bind(" that we could be using but are not.\nThe second is the amount our nation has built and is not damaged, and\nthe third is the amount \
that is available to us after bonuses, supply shortages\nand so on."))));
		add("supplies_icon", gui::Icon(Resources::texture("icon_supplies"), true).setPosition(480, -1)
			.setMessage(defaultTooltip.setText(gui::bind("Supplies\n", color) + gui::bind("Covers everything a modern army needs to operate\nexcept ")
				+ gui::bind("Fuel", color) + gui::bind(" - food rations, ammunition and weapons.\nThese are produced using ") +
				gui::bind("Industrial Capacity", color))));
		add("fuel_icon", gui::Icon(Resources::texture("icon_fuel"), true).setPosition(538, -1)
			.setMessage(defaultTooltip.setText(gui::bind("Fuel\n", color) + gui::bind("Petroleum, oil and lubricants. Refined from ") +
				gui::bind("Crude Oil", color) + fullstop + gui::bind("\nAll motorised units, aircraft and ships need these to\noperate effectively."))));
		add("money_icon", gui::Icon(Resources::texture("icon_money"), true).setPosition(591, -1)
			.setMessage(defaultTooltip.setText(gui::bind("Money\n", color) + gui::bind("Consists of reserves of gold, foreign exchange or work done, and is used\nmainly \
to purchase goods that are not produced at home from other\ncountries. Passively generated by our nation's ") + gui::bind("Industrial Capacity", color) +
				fullstop)));
		add("mp_icon", gui::Icon(Resources::texture("icon_manpower"), true).setPosition(664, -3)
			.setMessage(defaultTooltip.setText(gui::bind("Manpower\n", color) + gui::bind("Represents men of military age ready and able to form drafts\nfor our armies. \
They can be used to make new units as well as\nbring existing ones back to full strength."))));
		add("diplo_icon", gui::Icon(Resources::texture("icon_diplomacy"), true).setPosition(735, -5)
			.setMessage(defaultTooltip.setText(gui::bind("Diplomatic Influence\n", color) + gui::bind("Represents our pool of trained foreign service professionals,\nwhich we \
can use for diplomatic missions."))));
		add("espionage_icon", gui::Icon(Resources::texture("icon_espionage"), true).setPosition(778, -4)
			.setMessage(defaultTooltip.setText(gui::bind("Espionage\n", color) + gui::bind("Represents the total number of available spies and the infrastructure needed for\n\
supporting them. These people are well-trained in the underhanded art of espionage\nand will aid us in our attempts to subvert enemy countries and protect our own."))));
		add("officer_ratio_icon", gui::Icon(Resources::texture("icon_leadership"), true).setPosition(822, -4)
			.setMessage(defaultTooltip.setText(gui::bind("Officer Ratio\n", color) + gui::bind("If the ratio of active officers to enlisted soldiers falls below 100 %,\nour units will \
become increasingly likely to fall apart in combat\nshould they take too many losses."))));
		add("dissent_icon", gui::Icon(Resources::texture("icon_dissent"), true).setPosition(873, -3)
			.setMessage(defaultTooltip.setText(gui::bind("Dissent\n", color) + gui::bind("Represents the short-term unhappiness of our people. This can be reduced\nby increasing the supply \
of consumer goods to the civillian economy. If\nleft ignored, this will start to reduce support for the ruling party."))));
		add("unity_icon", gui::Icon(Resources::texture("icon_unity"), true).setPosition(913, -4)
			.setMessage(defaultTooltip.setText(gui::bind("National Unity\n", color) + gui::bind("Represents the willingness of our people to fight a war to the finish. The higher\nthis value is \
the longer we will fight before considering surrender."))));
		add("date", Date(font).setPosition(99, 29));
		add("nation_name", gui::TextArea("", font, 13).setColor(sf::Color(165, 169, 163)).setPosition(99, 47));

		auto resourceBreakdown([](const bEnd::Resource resource, const bEnd::ResourceDistributor::ResourceChangeCategory category, const gui::ColoredText& text)
		{
			const auto amount(bEnd::ResourceDistributor::get(bEnd::Nation::player).getResourceGain(resource, category));
			if (amount == 0.0f)
				return gui::bind("", sf::Color());
			else
			{
				std::stringstream ss;
				ss << std::fixed << std::setprecision(2) << amount;
				return gui::bind("\n") + gui::bind(ss.str(), amount > 0.0f ? sf::Color::Green : sf::Color::Red) + text;
			}
		});

		auto addResourceGauge([color, &defaultTooltip, fullstop, this, &resourceBreakdown, &font](const bEnd::Resource resource, const std::string& name, const sf::Vector2f& position)
		{
			add(name, gui::TextArea("", font, 12).setPosition(position).setUpdateFunction([color, resource]()
			{
				const auto& source(bEnd::ResourceDistributor::get(bEnd::Nation::player));
				const float amount(source.getResourceAmount(resource));
				return gui::bind(amount >= 100000 ? std::to_string(int(amount / 1000)) + " k" : std::to_string(int(amount)),
					source.getResourceGain(resource, bEnd::ResourceDistributor::Total) > 0 ? sf::Color::Green :
					(source.getResourceGain(resource, bEnd::ResourceDistributor::Total) == 0 ? color : sf::Color::Red));
			})
				.setMessage(defaultTooltip.setText(gui::bind("") + [resourceBreakdown, color, fullstop, resource]()
				{
					const auto amount(bEnd::ResourceDistributor::get(bEnd::Nation::player).getResourceGain(resource, bEnd::ResourceDistributor::Total));
					std::stringstream ss;
					ss << std::fixed << std::setprecision(2) << amount;
					return gui::bind("Changes by ", sf::Color::White) + gui::bind(ss.str(), amount > 0.0f ? sf::Color::Green : amount == 0 ? color : sf::Color::Red) +
						gui::bind(" every day.", sf::Color::White) +
						resourceBreakdown(resource, bEnd::ResourceDistributor::Generated, gui::bind(" generated in home territory.")) +
						resourceBreakdown(resource, bEnd::ResourceDistributor::Used, gui::bind(" used.")) +
						resourceBreakdown(resource, bEnd::ResourceDistributor::ResourceChangeCategory::Traded,
							gui::bind(bEnd::ResourceDistributor::get(bEnd::Nation::player).getResourceGain(resource, bEnd::ResourceDistributor::Traded) > 0 ?
								" traded for." : " traded away.")) +
						resourceBreakdown(resource, bEnd::ResourceDistributor::ConvertedTo, gui::bind(" converted to.")) + 
						resourceBreakdown(resource, bEnd::ResourceDistributor::ConvertedFrom, gui::bind(" converted from."));
				})));
		});

		addResourceGauge(bEnd::Energy, "energy", sf::Vector2f(118, 2));
		addResourceGauge(bEnd::Metal, "metal", sf::Vector2f(186, 2));
		addResourceGauge(bEnd::RareMaterials, "rare_mats", sf::Vector2f(250, 2));
		addResourceGauge(bEnd::CrudeOil, "oil", sf::Vector2f(306, 2));

		add("IC", gui::TextPane([]() 
		{
			const float amount(bEnd::ResourceDistributor::get(bEnd::Nation::player).getWastedIC());
			return gui::bind(std::to_string((unsigned short)(amount)), amount > 0 ? sf::Color::Red : sf::Color::Green);
		} + gui::bind("-", sf::Color(188, 183, 169)) + []()
		{
			const auto& source(bEnd::ResourceDistributor::get(bEnd::Nation::player));
			return gui::bind(std::to_string((unsigned short)(source.getBaseIC())),
				source.getICResourceBottleneck() == 1 ? sf::Color::Green : sf::Color::Red);
		} + gui::bind("-", sf::Color(188, 183, 169)) + []()
		{
			const auto& source(bEnd::ResourceDistributor::get(bEnd::Nation::player));
			return gui::bind(std::to_string((unsigned short)(source.getAvailableIC() * source.getICResourceBottleneck())),
				source.getICResourceBottleneck() == 1 ? sf::Color::Green : sf::Color::Red);
		}, font, 13).setPosition(383, 3));

		addResourceGauge(bEnd::Supplies, "supplies", sf::Vector2f(504, 2));
		addResourceGauge(bEnd::Fuel, "fuel", sf::Vector2f(558, 2));
		addResourceGauge(bEnd::Money, "money", sf::Vector2f(611, 2));

		add("manpower", gui::TextArea("", font, 12).setPosition(692, 2).setUpdateFunction([]()
		{
			return gui::bind(std::to_string(int(bEnd::ResourceDistributor::get(bEnd::Nation::player).getManpowerAmount())));
		})
			.setMessage(defaultTooltip.setText(gui::bind("Our army needs ") + gui::bind("0.0", color) + gui::bind(" manpower to reinforce, and we use ") +
				gui::bind("0.0", color) + gui::bind("\nevery day.\nMonthly gain: ") + [color]() 
				{
					std::stringstream ss;
					ss << std::fixed << std::setprecision(2) << bEnd::ResourceDistributor::get(bEnd::Nation::player).getManpowerGain();
					return gui::bind(ss.str(), color);
				})));

		add("diplo_infl", gui::TextArea("", font, 12).setPosition(762, 2).setUpdateFunction([]()
		{
			return gui::bind("0");
		}).setMessage(defaultTooltip.setText(gui::bind("") +
			[color, fullstop]()
		{
			const auto amount(bEnd::LeadershipDistributor::get(bEnd::Nation::player).getLeadershipAmount() *
				bEnd::LeadershipDistributor::get(bEnd::Nation::player).getLeadershipDistributionAmount(bEnd::LeadershipDistributor::ToDiplomacy));
			return gui::bind("Every day our pool of influence changes by ") +
				gui::bind(std::to_string(amount), amount > 0 ? sf::Color::Green : amount == 0 ? color : sf::Color::Red) + fullstop;
		})));

		add("espionage", gui::TextArea("", font, 12).setPosition(803, 2).setUpdateFunction([]()
		{
			const auto amount(bEnd::LeadershipDistributor::get(bEnd::Nation::player).getLeadershipAmount() *
				bEnd::LeadershipDistributor::get(bEnd::Nation::player).getLeadershipDistributionAmount(bEnd::LeadershipDistributor::ToEspionage));
			std::stringstream ss;
			ss << std::fixed << std::setprecision(1) << amount;
			return gui::bind(ss.str());
		})
			.setMessage(defaultTooltip.setText(gui::bind("This is the number of spies we currently train per day."))));

		add("pause", gui::CheckBox(gui::Button(gui::Icon(Resources::texture("pause"), true))
				.setName(std::move(gui::TextArea("Pause", font, 14).setPosition(-1, -3).setColor(sf::Color(255, 255, 255, 200))))
				.bindAction(gui::Released, []() { bEnd::TimeSystem::pause(); gui::AudioSystem::playSound(0); }),
			gui::Button(gui::Icon(Resources::texture("resume"), true))
				.setName(gui::TextArea("Resume", font, 14).setPosition(-1, -3).setColor(sf::Color(255, 255, 255, 200)))
			.bindAction(gui::Released, []() { bEnd::TimeSystem::resume(); gui::AudioSystem::playSound(0); }), bEnd::TimeSystem::isPaused())
			.setPosition(322, 26));

		add("spd_up", gui::Button(gui::Icon(Resources::texture("button_speedup"), true))
			.setPosition(280, 32)
			.bindAction(gui::Released, []() { bEnd::TimeSystem::increaseSpeed(); gui::AudioSystem::playSound(0); }));

		add("spd_dwn", gui::Button(gui::Icon(Resources::texture("button_speeddown"), true))
			.setPosition(299, 32)
			.bindAction(gui::Released, []() { bEnd::TimeSystem::decreaseSpeed(); gui::AudioSystem::playSound(0); }));

		add("spd", GameSpeedIndicator().setPosition(265, 35));

		{
			auto txtDefault(gui::TextArea("", font, 14).setColor(sf::Color(28, 29, 32, 230)).setPosition(0, -3));
			constexpr auto buttonY(27);
			add("dipl", gui::Button(gui::Icon(Resources::texture("topbarbutton_diplo"), true))
				.setName(txtDefault.setText("Diplomacy"))
				.setPosition(417, buttonY));
			add("prod", gui::Button(gui::Icon(Resources::texture("topbarbutton_prod"), true))
				.setName(txtDefault.setText("Production"))
				.setPosition(515, buttonY));
			add("tech", gui::Button(gui::Icon(Resources::texture("topbarbutton_tech"), true))
				.setName(txtDefault.setText("Technology"))
				.setPosition(613, buttonY));
			add("polit", gui::Button(gui::Icon(Resources::texture("topbarbutton_politics"), true))
				.setName(txtDefault.setText("Politics"))
				.setPosition(711, buttonY));
			add("intel", gui::Button(gui::Icon(Resources::texture("topbarbutton_intel"), true))
				.setName(txtDefault.setText("Intelligence"))
				.setPosition(809, buttonY));
			add("stats", gui::Button(gui::Icon(Resources::texture("topbarbutton_stats"), true))
				.setName(txtDefault.setText("Statistics"))
				.setPosition(907, buttonY)
				.setPredicates(gui::Button::PredicateArray{ []() { return false; } }));
		}

		shadow.setTexture(Resources::texture("topbar_shadow"));
		flagShadow.setTexture(Resources::texture("topbarflag_shadow"));
	}

	const bEnd::Tag& Topbar::getTarget() const
	{
		return target;
	}

	Topbar& Topbar::setTarget(const bEnd::Tag& tag)
	{
		target = tag;

		setBackgroundTexture(Resources::texture(Resources::textureExists("topbar_" + std::string(tag)) ? "topbar_" + std::string(tag) : "topbar_generic"));
		((gui::Icon&)at("flag")).setTexture(Nation::get(tag).getFlag());
		((gui::TextArea&)at("nation_name")).setText(Nation::get(tag).getName());

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

	Topbar::GameSpeedIndicator::GameSpeedIndicator()
		: gui::Icon(Resources::texture("speed")), paused(Resources::texture("paused")) {}

	Topbar::GameSpeedIndicator& Topbar::GameSpeedIndicator::setPosition(const float x, const float y)
	{
		gui::Icon::setPosition(x, y);
		paused.setPosition(x, y);
		return *this;
	}

	void Topbar::GameSpeedIndicator::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		if (bEnd::TimeSystem::isPaused())
		{
			target.draw(paused, states);
			return;
		}

		states.transform.translate(0, 4);
		for (auto it(bEnd::TimeSystem::getSpeed() + 1); it != 0; --it)
		{
			gui::Icon::draw(target, states);
			states.transform.translate(0, -3);
		}
	}
}
