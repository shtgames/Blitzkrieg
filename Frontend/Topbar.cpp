#include "Topbar.h"


#include <sstream>

namespace fEnd
{
	std::unordered_map<Topbar::imgResource, sf::Texture> Topbar::textures;
	const bool Topbar::ASSET_LOAD_SUCCESS = Topbar::loadAssets("topbar/assets.txt");

	Topbar::Topbar(const Nation& nation)
		: background(textures[BACKGROUND], true)
	{

	}

	const sf::FloatRect Topbar::getGlobalBounds() const
	{
	}

	const sf::Vector2f& Topbar::getPosition() const
	{
	}

	Topbar& Topbar::setPosition(const float, const float)
	{
	}

	const bool Topbar::input(const sf::Event &)
	{
		return false;
	}

	void Topbar::updateValues(const Nation&)
	{
	}

	void Topbar::draw(sf::RenderTarget&, sf::RenderStates)const
	{
	}

	const bool Topbar::loadAssets(const std::string&)
	{
	}
}
