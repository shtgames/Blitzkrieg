#include "utilities.h"

#include <cmath>
#include <algorithm>
#include <unordered_map>

#include <iostream>

#define PI 3.1416

namespace utl
{
	const bool pointsAreOnOneLine(const sf::Vector2s& A, const sf::Vector2s& B, const sf::Vector2s& C)
	{
		return !(A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));
	}

	const char angleType(const sf::Vector2s& A, const sf::Vector2s& B, const sf::Vector2s& C)
	{
		if (utl::pointsAreOnOneLine(A, B, C)) return -1;
		if ((A.x - B.x == 1 && A.y == B.y && C.x == B.x && C.y - B.y == 1) || (A.y - B.y == 1 && A.x == B.x && C.y == B.y && C.x - B.x == 1)) return 0;
		if ((A.x == B.x && B.y - A.y == 1 && C.y == B.y && B.x - C.x == 1) || (A.y == B.y && B.x - A.x == 1 && C.x == B.x && B.y - C.y == 1)) return 1;
		if ((A.x == B.x && B.y - A.y == 1 && C.y == B.y && C.x - B.x == 1) || (A.y == B.y && A.x - B.x == 1 && C.x == B.x && B.y - C.y == 1)) return 2;
		if ((B.x - A.x == 1 && A.y == B.y && C.x == B.x && C.y - B.y == 1) || (A.y - B.y == 1 && A.x == B.x && C.y == B.y && B.x - C.x == 1)) return 3;
		return -2;
	}

	void cullBorderTriangles(const std::vector<std::vector<sf::Color>>& pixels, const sf::Color& colorCode,
		std::vector<sf::Vector2s>& ProvinceContour, std::vector<sf::Vector2s>& borderTrianglesTarget)
	{
		for (auto it(ProvinceContour.begin()); it != ProvinceContour.end(); ++it)
		{
			const auto A((it == ProvinceContour.begin() ? ProvinceContour.end() : it) - 1),
				B(it),
				C(it == ProvinceContour.end() - 1 ? ProvinceContour.begin() : it + 1),
				prev((A == ProvinceContour.begin() ? ProvinceContour.end() : A) - 1),
				next(C == ProvinceContour.end() - 1 ? ProvinceContour.begin() : C + 1);
			if (angleType(*A, *B, *C) >= 0 && pixels.at((A->x + B->x + C->x) / 3).at((A->y + B->y + C->y) / 3) == colorCode &&
				!(pointsAreOnOneLine(*prev, *A, *B) && pointsAreOnOneLine(*B, *C, *next)) && (B->x - C->x != A->x - next->x ||
					B->y - C->y != A->y - next->y) && (B->x - A->x != C->x - prev->x || B->y - A->y != C->y - prev->y))
			{
				borderTrianglesTarget.push_back(*A);
				borderTrianglesTarget.push_back(*B);
				borderTrianglesTarget.push_back(*C);
				it = ProvinceContour.erase(it);
			}
		}
	}

	bool onSegment(const sf::Vector2f& p, const sf::Vector2f& q, const sf::Vector2f& r)
	{
		if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
			q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
			return true;
		return false;
	}

	int orientation(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& r)
	{
		int val = (p2.y - p1.y) * (r.x - p2.x) -
			(p2.x - p1.x) * (r.y - p2.y);

		if (val == 0) return 0;

		return (val > 0) ? 1 : 2;
	}

	const bool linesIntersect(const sf::Vector2f& p1, const sf::Vector2f& q1, const sf::Vector2f& p2, const sf::Vector2f& q2)
	{
		int o1 = orientation(p1, q1, p2);
		int o2 = orientation(p1, q1, q2);
		int o3 = orientation(p2, q2, p1);
		int o4 = orientation(p2, q2, q1);

		if (o1 != o2 && o3 != o4)
			return true;

		if (o1 == 0 && onSegment(p1, p2, q1)) return true;
		if (o2 == 0 && onSegment(p1, q2, q1)) return true;
		if (o3 == 0 && onSegment(p2, p1, q2)) return true;
		if (o4 == 0 && onSegment(p2, q1, q2)) return true;

		return false;
	}

	const bool pointIsInsidePolygon(const sf::VertexArray& points, const unsigned int indexBegin, const unsigned int indexEnd, const sf::Vector2f& point)
	{
		sf::Vector2u min((unsigned short)(-1), (unsigned short)(-1)), max(0, 0);
		for (auto i(indexBegin); i != indexEnd; ++i)
		{
			if (points[i].position.x > max.x) max.x = points[i].position.x;
			if (points[i].position.y > max.y) max.y = points[i].position.y;
			if (points[i].position.x < min.x) min.x = points[i].position.x;
			if (points[i].position.y < min.y) min.y = points[i].position.y;
		}
		if (point.x < min.x || point.x > max.x || point.y < min.y || point.y > max.y) return false;

		sf::Vector2f ray(-1, point.y);
		unsigned short intersections(0);
		for (auto i(indexBegin); i != indexEnd; ++i)
			if (linesIntersect(points[i].position, points[i + 1].position, ray, point)) intersections++;

		return intersections % 2;
	}

	const bool pointIsInsideTriangle(const sf::Vector2s& A, const sf::Vector2s& B, const sf::Vector2s& C, const sf::Vector2s& point)
	{
		const float alpha = ((B.y - C.y) * (point.x - C.x) + (C.x - B.x) * (point.y - C.y)) / ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y)),
			beta = ((C.y - A.y) * (point.x - C.x) + (A.x - C.x) * (point.y - C.y)) / ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y)),
			gamma = 1.0f - alpha - beta;
		return alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f;
	}

	const bool pointIsInsideTriangle(const sf::Vector2f& A, const sf::Vector2f& B, const sf::Vector2f& C, const sf::Vector2f& point)
	{
		const float alpha = ((B.y - C.y) * (point.x - C.x) + (C.x - B.x) * (point.y - C.y)) / ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y)),
			beta = ((C.y - A.y) * (point.x - C.x) + (A.x - C.x) * (point.y - C.y)) / ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y)),
			gamma = 1.0f - alpha - beta;
		return alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f;
	}

	std::vector<std::vector<sf::Color>> imageToPixelArray(const sf::Image& source)
	{
		auto pixels(source.getPixelsPtr());
		std::vector<std::vector<sf::Color>> returnValue;

		for (unsigned short i = 0, end = source.getSize().x; i != end; i++)
		{
			returnValue.emplace_back();
			for (unsigned short i1 = 0, end1 = source.getSize().y; i1 != end1; i1++)
			{
				const unsigned int index((i + i1 * end) * 4);
				returnValue.back().push_back(sf::Color(pixels[index], pixels[index + 1], pixels[index + 2], pixels[index + 3]));
			}
		}

		return std::move(returnValue);
	}

	void floodFillColorChange(std::vector<std::vector<sf::Color>>& pixels, unsigned short x, unsigned short y, const sf::Color& previousColor,
		const sf::Color& newColor)
	{
		std::vector<sf::Vector2s> pointVector;
		pointVector.emplace_back(x, y);
		while (!pointVector.empty())
		{
			sf::Vector2s point(pointVector.back());
			pointVector.pop_back();
			if (pixels.at(point.x + 1).at(point.y) == previousColor)
				pointVector.emplace_back(point.x + 1, point.y);

			if (pixels.at(point.x - 1).at(point.y) == previousColor)
				pointVector.emplace_back(point.x - 1, point.y);

			if (pixels.at(point.x).at(point.y + 1) == previousColor)
				pointVector.emplace_back(point.x, point.y + 1);

			if (pixels.at(point.x).at(point.y - 1) == previousColor)
				pointVector.emplace_back(point.x, point.y - 1);

			pixels.at(point.x).at(point.y) = newColor;
		}
	}

	const sf::Vector2s findStartingPixel(const std::vector<std::vector<sf::Color>>& pixels, const sf::Color& color)
	{
		for (auto it(pixels.begin()), end(pixels.end()); it != end; ++it)
			for (auto it1(it->begin()), end1(it->end()); it1 != end1; ++it1)
				if (*it1 == color)
					return sf::Vector2s(std::distance(pixels.begin(), it), std::distance(it->begin(), it1));
		return sf::Vector2s(-1, -1);
	}

	const std::vector<std::vector<sf::Vector2s>> marchingSquares(std::vector<std::vector<sf::Color>>& pixels, const sf::Color& color)
	{
		class Cell
		{
		public:
			Cell(const std::vector<std::vector<sf::Color>>& pixels, const sf::Color& color) : map(pixels), criteria(color) {}
			~Cell() = default;
			const sf::Vector2s getPosition() { return D; }
			void setPosition(unsigned short x, unsigned short y) {
				A = sf::Vector2s(x - 1, y - 1);
				B = sf::Vector2s(x, y - 1);
				C = sf::Vector2s(x - 1, y);
				D = sf::Vector2s(x, y);
				AValue = map.at(x - 1).at(y - 1) == criteria;
				BValue = map.at(x).at(y - 1) == criteria;
				CValue = map.at(x - 1).at(y) == criteria;
				DValue = map.at(x).at(y) == criteria;
			}
			void setPosition(sf::Vector2s newPosition) { setPosition(newPosition.x, newPosition.y); }
			const sf::Vector2s nextMove()
			{
				if (AValue && BValue && !CValue && !DValue) setPosition(getPosition().x + 1, getPosition().y);
				else if (!AValue && !BValue && CValue && DValue) setPosition(getPosition().x - 1, getPosition().y);
				else if (!AValue && BValue && !CValue && !DValue) setPosition(getPosition().x + 1, getPosition().y);
				else if (AValue && BValue && CValue && !DValue) setPosition(getPosition().x + 1, getPosition().y);
				else if (AValue && !BValue && !CValue && !DValue) setPosition(getPosition().x, getPosition().y - 1);
				else if (AValue && !BValue && CValue && DValue) setPosition(getPosition().x, getPosition().y - 1);
				else if (!AValue && BValue && !CValue && DValue) setPosition(getPosition().x, getPosition().y + 1);
				else if (AValue && !BValue && CValue && !DValue) setPosition(getPosition().x, getPosition().y - 1);
				else if (!AValue && BValue && CValue && DValue) setPosition(getPosition().x - 1, getPosition().y);
				else if (!AValue && !BValue && !CValue && DValue) setPosition(getPosition().x, getPosition().y + 1);
				else if (!AValue && !BValue && CValue && !DValue) setPosition(getPosition().x - 1, getPosition().y);
				else if (AValue && BValue && !CValue && DValue) setPosition(getPosition().x, getPosition().y + 1);
				else if (AValue && !BValue && !CValue && DValue) setPosition(getPosition().x, getPosition().y - 1);
				else if (!AValue && BValue && CValue && !DValue) setPosition(getPosition().x - 1, getPosition().y);
				return getPosition();
			}

		private:
			const std::vector<std::vector<sf::Color>>& map;
			sf::Vector2s A, B, C, D;
			bool AValue, BValue, CValue, DValue;
			const sf::Color criteria;
		};

		std::vector<std::vector<sf::Vector2s>> returnValue;
		std::vector<sf::Vector2s> previousPointColors;
		while (true)
		{
			std::vector<sf::Vector2s> points{ findStartingPixel(pixels, color) };
			if (points.front() == sf::Vector2s(-1, -1))break;

			Cell cell(pixels, color);
			cell.setPosition(points.front());
			points.push_back(cell.nextMove());

			while (cell.nextMove() != points.front())
			{
				points.push_back(cell.getPosition());
				if (points.size() > 5000)
				{
					points.clear();
					break;
				}
			}

			floodFillColorChange(pixels, points.front().x, points.front().y, color, sf::Color(0, 0, 0, 0));
			previousPointColors.emplace_back(points.front());

			if (points.empty()) continue;
			else returnValue.push_back(points);
		}

		for (const auto& it : previousPointColors)
			floodFillColorChange(pixels, it.x, it.y, sf::Color(0, 0, 0, 0), color);

		return returnValue;
	}

	const std::vector<sf::Vector2s> simplifyShape(const std::vector<sf::Vector2s>& points)
	{
		std::vector<sf::Vector2s> returnValue;

		for (auto begin(points.begin()), end(points.end()), it(begin); it != end; ++it)
		{
			const sf::Vector2s A = *((it == begin ? end : it) - 1),
				B = *it,
				C = *(it == end - 1 ? begin : it + 1);
			if (!pointsAreOnOneLine(A, B, C)) returnValue.push_back(B);
		}

		return returnValue;
	}

	void tesselateShape(const std::vector<sf::Vector2s>& source, std::vector<sf::Vector2s>& target)
	{
		if (source.size() <= 2) return;

		std::vector<std::vector<sf::Vector2s>> polygons;
		polygons.push_back(source);
		target.reserve(target.size() + (source.size() - 2) * 3);

		while (!polygons.empty())
		{
			if (polygons.back().size() == 3)
			{
				target.emplace_back(polygons.back().at(0));
				target.emplace_back(polygons.back().at(1));
				target.emplace_back(polygons.back().at(2));
				polygons.pop_back();
				continue;
			}
			else if (polygons.back().size() < 3)
			{
				polygons.pop_back();
				continue;
			}

			std::vector<sf::Vector2s>::const_iterator A(polygons.back().begin());
			const std::vector<sf::Vector2s>::const_iterator begin(A), end(polygons.back().end());
			for (auto it(A); it != end; ++it)
				if (it->x < A->x)
					A = it;

			std::vector<sf::Vector2s>::const_iterator B((A == begin ? end : A) - 1),
				C((A == end - 1 ? begin : A + 1));

			bool pointWasInside(false);
			for (auto it(begin); it != end; ++it)
			{
				if (it == A || it == B || it == C) continue;
				if (pointIsInsideTriangle(*A, *B, *C, *it))
				{
					pointWasInside = true;
					C = it;
					it = begin;
				}
			}

			target.emplace_back(*A);
			target.emplace_back(*B);
			target.emplace_back(*C);
			if (!pointWasInside)
				polygons.back().erase(A);
			else
			{
				std::vector<sf::Vector2s> polygon;
				for (; A != C + 1; ++A)
				{
					if (A == end) A = begin;
					polygon.emplace_back(*A);
				}
				polygons.emplace_back(std::move(polygon));
				for (; C != B + 1; ++C)
				{
					if (C == end) C = begin;
					polygon.emplace_back(*C);
				}
				polygons.emplace_back(std::move(polygon));
				polygons.erase(polygons.begin() + (polygons.size() - 3));
			}
		}
	}

	unsigned int length(const sf::Vector2f& a, const sf::Vector2f& b)
	{
		return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	}

	std::pair<float, float> getFunc(const sf::Vector2f& A, const sf::Vector2f& B)
	{
		const auto first((B.y - A.y) / (B.x - A.x));
		return std::make_pair(first, A.y - A.x * first);
	}

	const bool haveCommonSegment(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& c, const sf::Vector2f& d)
	{
		if ((a == c && b == d) || (a == d && b == c)) return true;

		if (a.x == b.x && c.x == d.x && a.x == c.x)
			return (b.y < c.y && b.y > d.y) || (b.y > c.y && b.y < d.y)
				|| (a.y < c.y && a.y > d.y) || (a.y > c.y && a.y < d.y);
		else if (a.x != b.x && c.x != d.x && getFunc(a, b) == getFunc(c, d))
			return (b.x < c.x && b.x > d.x) || (b.x > c.x && b.x < d.x)
				|| (a.x < c.x && a.x > d.x) || (a.x > c.x && a.x < d.x);

		return false;
	}

	unsigned short distanceBetweenPoints(const sf::Vector2f& a, const sf::Vector2f& b)
	{
		return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
	}
}
