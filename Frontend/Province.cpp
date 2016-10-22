#include "Province.h"

#include <unordered_map>

namespace fEnd
{
	const bool isProvinceAnIsland(const std::vector<std::vector<sf::Color>>& pixels, const std::vector<sf::Vector2s> contour, const sf::Color& colorCode)
	{
		if (pixels.empty() || contour.empty()) return false;

		std::vector<sf::Vector2s> pointVector;

		for (const auto& it : contour)
			if (pixels.at(it.x).at(it.y) == colorCode)
			{
				pointVector.push_back(it);
				break;
			}

		std::unique_ptr<sf::Color> firstBorderColor(nullptr);
		std::unordered_map<unsigned short, std::unordered_map<unsigned short, bool>> flag;

		while (!pointVector.empty())
		{
			const sf::Vector2s point(pointVector.back());
			pointVector.pop_back();

			if (!flag[point.x + 1][point.y])
			{
				if (pixels.at(point.x + 1).at(point.y) == colorCode)
					pointVector.emplace_back(point.x + 1, point.y);
				else if (firstBorderColor && *firstBorderColor != pixels.at(point.x + 1).at(point.y)) return false;
				else firstBorderColor.reset(new sf::Color(pixels.at(point.x + 1).at(point.y)));
			}

			if (!flag[point.x - 1][point.y])
			{
				if (pixels.at(point.x - 1).at(point.y) == colorCode)
					pointVector.emplace_back(point.x - 1, point.y);
				else if (firstBorderColor && *firstBorderColor != pixels.at(point.x - 1).at(point.y)) return false;
				else firstBorderColor.reset(new sf::Color(pixels.at(point.x - 1).at(point.y)));
			}

			if (!flag[point.x][point.y + 1])
			{
				if (pixels.at(point.x).at(point.y + 1) == colorCode)
					pointVector.emplace_back(point.x, point.y + 1);
				else if (firstBorderColor && *firstBorderColor != pixels.at(point.x).at(point.y + 1)) return false;
				else firstBorderColor.reset(new sf::Color(pixels.at(point.x).at(point.y + 1)));
			}

			if (!flag[point.x][point.y - 1])
			{
				if (pixels.at(point.x).at(point.y - 1) == colorCode)
					pointVector.emplace_back(point.x, point.y - 1);
				else if (firstBorderColor && *firstBorderColor != pixels.at(point.x).at(point.y - 1)) return false;
				else firstBorderColor.reset(new sf::Color(pixels.at(point.x).at(point.y - 1)));
			}

			flag[point.x][point.y] = true;
		}

		return true;
	}
	void cullBorderTriangles(std::vector<sf::Vector2s>& ProvinceContour)
	{
		if (ProvinceContour.size() <= 4) return;

		for (auto it(ProvinceContour.begin()); it != ProvinceContour.end(); ++it)
		{
			const auto A((it == ProvinceContour.begin() ? ProvinceContour.end() : it) - 1),
				B(it),
				C(it == ProvinceContour.end() - 1 ? ProvinceContour.begin() : it + 1),
				prev((A == ProvinceContour.begin() ? ProvinceContour.end() : A) - 1),
				next(C == ProvinceContour.end() - 1 ? ProvinceContour.begin() : C + 1);
			if (utl::angleType(*A, *B, *C) >= 0 && !(utl::pointsAreOnOneLine(*prev, *A, *B) &&
				utl::pointsAreOnOneLine(*B, *C, *next)) && (B->x - C->x != A->x - next->x || B->y - C->y != A->y - next->y) &&
				(B->x - A->x != C->x - prev->x || B->y - A->y != C->y - prev->y))
				it = ProvinceContour.erase(it);

			if (it == ProvinceContour.end()) break;
		}
	}
	void Province::traceShape(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
		std::vector<sf::Vector2s>& borderTrianglesTarget, std::vector<std::vector<sf::Vector2s>>& contourPointsTarget)
	{
		auto points(std::move(utl::marchingSquares(pixels, colorCode)));

		for (auto& it : points)
		{
			if (it.empty()) continue;

			isProvinceAnIsland(pixels, it, colorCode) ? cullBorderTriangles(it) :
				utl::cullBorderTriangles(pixels, colorCode, it, borderTrianglesTarget);
			contourPointsTarget.emplace_back(it);
		}
	}
}
