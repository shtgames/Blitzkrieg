#include "MapTransitionAnimation.h"

#include "../Frontend.hpp"

namespace fEnd
{
	const std::string stripeShaderCode =
		"uniform sampler2D mask, first, second;\
		uniform float zoomFactor, amount;\
		\
		vec4 highlight(vec4 color)\
		{\
			color.r += 0.15f * (1.0f - color.r);\
			color.g += 0.15f * (1.0f - color.g);\
			color.b += 0.15f * (1.0f - color.b);\
			\
			return color;\
		}\
		\
		void main()\
		{\
			gl_FragColor.rgb = (gl_Color *\
				mix( texture2D(first, gl_TexCoord[0].xy / (2 * zoomFactor)), texture2D(second, gl_TexCoord[0].xy ), amount )).rgb;\
			gl_FragColor.a = texture2D(mask, gl_TexCoord[0].xy).a;\
		}";

	const std::string provinceFillTextureTransition =
		"uniform sampler2D first, second;\
		uniform float zoomFactor, amount;\
		uniform vec4 highlightColor;\
		\
		vec4 highlight(vec4 color)\
		{\
			color.r += 0.15f * (1.0f - color.r);\
			color.g += 0.15f * (1.0f - color.g);\
			color.b += 0.15f * (1.0f - color.b);\
			\
			return color;\
		}\
		\
		void main()\
		{\
			gl_FragColor = (gl_Color == highlightColor ? highlight(gl_Color) : gl_Color) *\
				mix( texture2D(first, gl_TexCoord[0].xy / (2 * zoomFactor)), texture2D(second, gl_TexCoord[0].xy ), amount );\
		}";


	MapTransitionAnimation::MapTransitionAnimation()
	{
		if (!sf::Shader::isAvailable())
			console.print("Unable to load shaders. Reason: System does not support Shader Model 3.0. Some elements may be rendered incorrectly.");
		else if (!fill.loadFromMemory(provinceFillTextureTransition, sf::Shader::Fragment) ||
			!stripes.loadFromMemory(stripeShaderCode, sf::Shader::Fragment))
			console.print("Failed to load shaders: Some elements may be rendered incorrectly.");
		else console.print("Shaders loaded successfully.");
	}

	void MapTransitionAnimation::step() const
	{
		amount += (direction ? 1 : -1) / (getAnimationFPS() * getAnimationDuration());
		if (amount > 1.0f) amount = 1.0f;
		else if (amount < 0.0f) amount = 0.0f;
		fill.setUniform("amount", amount);
		stripes.setUniform("amount", amount);
	}

	void MapTransitionAnimation::setTextures(const sf::Texture& first)
	{
		fill.setUniform("first", first);
		fill.setUniform("second", sf::Shader::CurrentTexture);
		stripes.setUniform("first", first);
		stripes.setUniform("second", sf::Shader::CurrentTexture);
	}

	void MapTransitionAnimation::setMask(const sf::Texture& mask)
	{
		stripes.setUniform("mask", mask);
	}

	void MapTransitionAnimation::setFactor(const float factor)
	{
		fill.setUniform("zoomFactor", factor);
		stripes.setUniform("zoomFactor", factor);
	}

	void MapTransitionAnimation::setHighlightedColor(const sf::Color& color)
	{
		fill.setUniform("highlightColor", sf::Glsl::Vec4(color));
	}

	const sf::Shader& MapTransitionAnimation::getFillShader() const
	{
		return fill;
	}

	const sf::Shader& MapTransitionAnimation::getStripeShader() const
	{
		return stripes;
	}
}
