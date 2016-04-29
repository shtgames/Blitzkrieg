#ifndef REGION_FRONTEND
#define REGION_FRONTEND

#include <GUI/Interactive.h>
#include <GUI/Internals.h>
#include <GUI/FadeAnimation.h>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Window/Event.hpp>

#include <atomic>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <mutex>
#include <queue>

#include "Camera.h"
#include "utilities.h"

namespace fEnd
{
	class Map final : public gui::Interactive
	{
	public:
		Map(const Map& copy) = default;
		Map(Map&&) = default;
		Map() = default;
		~Map() = default;

		std::unique_ptr<Interactive> copy()const override;
		std::unique_ptr<Interactive> move()override;

		const bool input(const sf::Event& event)override; 
		
		const sf::FloatRect getGlobalBounds() const override;
		const sf::Vector2f& getPosition() const override; 
		
		Map& setPosition(const float x, const float y)override;

		static const sf::Vector2s& size();

		static void initialize();
		static void addRegionNeedingColorUpdate(const unsigned short regionID);
		static void updateAllRegionColors();
		static void loadRegions();
		static void updateRegionVisuals(const sf::Vector2s& resolution);
		static void loadResources();
		static void launchRegionUpdateThread();
		static void stopRegionUpdateThread();

		static Camera camera;

	private:
		struct Region
		{
			void traceShape(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
				std::vector<sf::Vector2s>& borderTrianglesTarget, std::vector<std::vector<sf::Vector2s>>& contourPointsTarget);
			std::string name;
			unsigned int indexBegin, indexEnd;
		};

		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

		static gui::FadeAnimation animation;

		static std::unordered_map<unsigned short, Region> regions;

		static sf::VertexArray stripesBuffer[2], landBuffer[2], seaBuffer[2];
		static std::atomic<bool> drawableBufferSet, vertexArraysVisibilityNeedsUpdate, updateThreadLaunched;
		static std::queue<unsigned short> regionsNeedingColorUpdate;
		static std::mutex colorUpdateQueueLock;

		static sf::Vector2s mapSize;
		static sf::RenderTexture land;
		static sf::VertexArray oceanGradient, provinceStripes, landProvinces, seaProvinces;

		static sf::Texture mapTile, stripes;
		static std::pair<sf::Shader, sf::Vector2f> border;
		static sf::Shader stripesShader;
		
		static void clickCheck(const sf::Vector2s& point);
		static void updateVertexArrays();

		static void assignBorderTriangles(std::vector<sf::Vector2s>& unassignedTriangles,
			std::map<unsigned short, std::vector<std::vector<sf::Vector2s>>>& provinceContours);
		static void createStripesTexture(sf::Texture& targetTexture, const float size = 64.0f, const float stripeWidth = 0.4f);
		static void createProvinceCache();
		static void loadProvinceCache();
	};
}

#endif