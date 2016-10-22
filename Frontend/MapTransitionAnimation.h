#ifndef MAP_TRANSITION_ANIMATION
#define MAP_TRANSITION_ANIMATION

#include <GUI/Animation.h>

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace fEnd
{
	class MapTransitionAnimation final : public gui::Animation
	{
		mutable sf::Shader fill, stripes;
		mutable float amount = 0;

	public:
		MapTransitionAnimation();

		void step() const override;

		void setTextures(const sf::Texture& first);
		void setMask(const sf::Texture& mask);
		void setFactor(const float factor);
		void setHighlightedColor(const sf::Color& color);

		const sf::Shader& getFillShader() const;
		const sf::Shader& getStripeShader() const;

		bool direction = 0;
	};
}

#endif