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

#include "../Backend/Region.h"
#include "Camera.h"
#include "utilities.h"

namespace fEnd
{
	class Map final : public gui::Interactive
	{
		friend class Region;
	public:
		class Region
		{
			friend class Map;
		public:
			std::string name;
			std::atomic<bool> sea = false, highlighted = false, visible = false;
		private:
			void traceShape(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
				std::vector<sf::Vector2s>& borderTrianglesTarget, std::vector<std::vector<sf::Vector2s>>& contourPointsTarget);

			std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> indexRange;
		};

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

		static Region& get(const sf::Uint16 regionID);
		static void initialize();
		static void addRegionNeedingColorUpdate(const sf::Uint16 regionID);
		static void updateAllRegionColors();
		static void loadRegions();
		static void updateRegionVisuals(const sf::Vector2s& resolution);
		static void loadResources();
		static void launchRegionUpdateThread();
		static void stopRegionUpdateThread();
		static const sf::FloatRect getViewBounds();
		static void setViewPosition(const float x, const float y);
		
		static std::unique_ptr<sf::Uint16> target;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

		static Camera camera;
		static gui::FadeAnimation animation;

		static std::unordered_map<sf::Uint16, Region> regions;

		static sf::VertexArray stripesBuffer[2], fillBuffer[2], contourBuffer[2];
		static volatile std::atomic<bool> drawableBufferSet, vertexArraysVisibilityNeedsUpdate, updateThreadLaunched;
		static std::queue<sf::Uint16> regionsNeedingColorUpdate;
		static std::mutex colorUpdateQueueLock;

		static sf::Vector2s mapSize;
		static sf::VertexArray oceanGradient, provinceStripes, provinceFill, provinceContours;

		static sf::Texture mapTile, terrain, sea, stripes;
		
		static const sf::Uint16 clickCheck(sf::Vector2s point);
		static void updateVertexArrays();
		static void processUpdateQueue();

		static void assignBorderTriangles(std::vector<sf::Vector2s>& unassignedTriangles,
			std::map<sf::Uint16, std::vector<std::vector<sf::Vector2s>>>& provinceContours);
		static void createStripesTexture(sf::Texture& targetTexture, const float size = 64.0f, const float stripeWidth = 0.4f);
		static void createProvinceCache();
		static void loadProvinceCache();
	};
}

#endif