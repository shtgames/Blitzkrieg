#ifndef Province_FRONTEND
#define Province_FRONTEND

#include <GUI/Interactive.h>
#include <GUI/Internals.h>
#include <GUI/FadeAnimation.h>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>

#include <atomic>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <mutex>
#include <queue>

#include "../Backend/Province.h"

#include "Province.h"
#include "Camera.h"
#include "MapTransitionAnimation.h"
#include "utilities.h"

namespace fEnd
{
	class Map final : public gui::Interactive
	{
		friend class Province;
		friend class MapLoader;
	public:
		Map(const Map& copy) = default;
		Map(Map&&) = default;
		Map() = default;
		~Map() = default;

		static void initialise();
		static void terminate();

		std::unique_ptr<Interactive> copy()const override;
		std::unique_ptr<Interactive> move()override;
		
		static const sf::FloatRect getViewBounds();
		const sf::FloatRect getGlobalBounds() const override;
		const sf::Vector2f& getPosition() const override;
		static Province& get(const unsigned short ProvinceID);
		static const sf::Vector2s& size();

		static void setViewPosition(const float x, const float y);
		Map& setPosition(const float x, const float y)override;

		static void select(const unsigned short id);
		static void deselect();

		const bool input(const sf::Event& event)override;

		static void addProvinceNeedingColorUpdate(const unsigned short ProvinceID);
		static void updateAllProvinceColors();
		static void updateProvinceVisuals(const sf::Vector2s& resolution);
		
		static void waitForColorUpdate();

		static const std::unique_ptr<unsigned short>& target;

	private:
		static const unsigned short clickCheck(sf::Vector2s point);
		void draw(sf::RenderTarget& target, sf::RenderStates states)const override;

		static void updateVertexArrays();
		static void processUpdateQueue();

		static Camera camera;

		static gui::FadeAnimation outlineFadeAnimation;
		static std::unique_ptr<MapTransitionAnimation> textureTransitionAnimation;

		static std::unordered_map<unsigned short, Province> provinces;
		
		static sf::Vector2s mapSize;

		static sf::VertexArray oceanGradient, provinceStripes, provinceFill, provinceContours;
		static sf::VertexArray stripesBuffer[2], fillBuffer[2], contourBuffer[2];
		static volatile std::atomic<bool> drawableBufferSet, vertexArraysVisibilityNeedsUpdate;

		static std::queue<unsigned short> provincesNeedingColorUpdate;
		static std::mutex colorUpdateQueueLock/*, provincesLock*/;

		static std::unique_ptr<unsigned short> m_target;

		static sf::Texture tile, terrain, sea, stripes;

		static std::atomic<bool> m_terminate;
		static std::unique_ptr<std::thread> updateThread;
	};
}

#endif