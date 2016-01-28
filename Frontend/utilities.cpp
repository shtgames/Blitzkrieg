#include "utilities.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#define PI 3.1416

const bool utl::threePointsMakeALine(const sf::Vector2f& A, const sf::Vector2f& B, const sf::Vector2f& C)
{
	return !(A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));
}

const char utl::angleType(const sf::Vector2f& A, const sf::Vector2f& B, const sf::Vector2f& C)
{
	if (utl::threePointsMakeALine(A, B, C)) return -1;
	else if ((A.x - B.x == 1 && A.y == B.y && C.x == B.x && C.y - B.y == 1) || (A.y - B.y == 1 && A.x == B.x && C.y == B.y && C.x - B.x == 1)) return 0;
	else if ((A.x == B.x && B.y - A.y == 1 && C.y == B.y && B.x - C.x == 1) || (A.y == B.y && B.x - A.x == 1 && C.x == B.x && B.y - C.y == 1)) return 1;
	else if ((A.x == B.x && B.y - A.y == 1 && C.y == B.y && C.x - B.x == 1) || (A.y == B.y && A.x - B.x == 1 && C.x == B.x && B.y - C.y == 1)) return 2;
	else if ((B.x - A.x == 1 && A.y == B.y && C.x == B.x && C.y - B.y == 1) || (A.y - B.y == 1 && A.x == B.x && C.y == B.y && B.x - C.x == 1)) return 3;
	else return -2;
}

const std::vector<sf::Vector2f> utl::cullBorderTriangles(const std::vector<sf::Vector2f>& shape, sf::VertexArray& targetVArray, const sf::Image& map, const sf::Color& provColor)
{
	std::vector<sf::Vector2f> returnValue;
	for (std::vector<sf::Vector2f>::const_iterator it = shape.begin(), end = shape.end(); it != end; ++it)
	{
		const sf::Vector2f A = it - 1 >= shape.begin() ? *(it - 1) : *(it - 1 + shape.size()), B = *it, C = it + 1 < end ? *(it + 1) : *(it + 1 - shape.size()),
			D = it + 2 < shape.end() ? *(it + 2) : *(it + 2 - shape.size()), E = it - 2 >= shape.begin() ? *(it - 2) : *(it - 2 + shape.size());
		const char currentAngleType = angleType(A, B, C), nextAngleType = angleType(B, C, D), previousAngleType = angleType(E, A, B);
		if (currentAngleType == -1)
		{
			if (!utl::arrayContainsElement(returnValue, B)) returnValue.push_back(B);
		}
		else if (currentAngleType == 0)
		{
			if (map.getPixel(B.x, B.y) == provColor && ((nextAngleType == 1 && previousAngleType != 2 && previousAngleType != 3) || (previousAngleType == 1 && nextAngleType != 2 && nextAngleType != 3)))
			{
				targetVArray.append(sf::Vector2f(A));
				targetVArray.append(sf::Vector2f(B));
				targetVArray.append(sf::Vector2f(C));
			}
			else if (!utl::arrayContainsElement(returnValue, B)) returnValue.push_back(B);
		}
		else if (currentAngleType == 1)
		{
			if (map.getPixel(B.x - 1, B.y - 1) == provColor && ((nextAngleType == 0 && previousAngleType != 2 && previousAngleType != 3) || (previousAngleType == 0 && nextAngleType != 2 && nextAngleType != 3)))
			{
				targetVArray.append(sf::Vector2f(A));
				targetVArray.append(sf::Vector2f(B));
				targetVArray.append(sf::Vector2f(C));
			}
			else if (!utl::arrayContainsElement(returnValue, B)) returnValue.push_back(B);
		}
		else if (currentAngleType == 2)
		{
			if (map.getPixel(B.x, B.y - 1) == provColor && ((nextAngleType == 3 && previousAngleType != 0 && previousAngleType != 1) || (previousAngleType == 3 && nextAngleType != 0 && nextAngleType != 1)))
			{
				targetVArray.append(sf::Vector2f(A));
				targetVArray.append(sf::Vector2f(B));
				targetVArray.append(sf::Vector2f(C));
			}
			else if (!utl::arrayContainsElement(returnValue, B)) returnValue.push_back(B);
		}
		else if (currentAngleType == 3)
		{
			if (map.getPixel(B.x - 1, B.y) == provColor && ((nextAngleType == 2 && previousAngleType != 0 && previousAngleType != 1) || (previousAngleType == 2 && nextAngleType != 0 && nextAngleType != 1)))
			{
				targetVArray.append(sf::Vector2f(A));
				targetVArray.append(sf::Vector2f(B));
				targetVArray.append(sf::Vector2f(C));
			}
			else if (!utl::arrayContainsElement(returnValue, B)) returnValue.push_back(B);
		}
	}
	return returnValue;
}

void utl::floodFillColorChange(sf::Image& _map, int x, int y, sf::Color old_color, sf::Color new_color)
{
	std::vector<sf::Vector2i>* vector_r = new std::vector < sf::Vector2i >;
	vector_r->push_back(sf::Vector2i(x, y));
	while (!vector_r->empty())
	{
		sf::Vector2i a = vector_r->back();
		vector_r->pop_back();
		if (_map.getPixel(a.x + 1, a.y) == old_color)
		{
			vector_r->push_back(sf::Vector2i(a.x + 1, a.y));
		}

		if (_map.getPixel(a.x - 1, a.y) == old_color)
		{
			vector_r->push_back(sf::Vector2i(a.x - 1, a.y));
		}

		if (_map.getPixel(a.x, a.y + 1) == old_color)
		{
			vector_r->push_back(sf::Vector2i(a.x, a.y + 1));
		}

		if (_map.getPixel(a.x, a.y - 1) == old_color)
		{
			vector_r->push_back(sf::Vector2i(a.x, a.y - 1));
		}
		_map.setPixel(a.x, a.y, new_color);
	}
}

const sf::Vector2f utl::findStartingPixel(const sf::Image& _map, const sf::Color& _color)
{
	for (unsigned short i = 0; i < _map.getSize().x; i++)
	{
		for (unsigned short j = 0; j < _map.getSize().y; j++)
		{
			if (_map.getPixel(i, j) == _color)
			{
				return sf::Vector2f(i, j);
			}
		}
	}
	return sf::Vector2f(-1, -1);
}

const std::vector<std::vector<sf::Vector2f>> utl::marchingSquares(sf::Image& _map, const sf::Color& _color)
{
	class cell
	{
	public:
		cell(const sf::Image& map_, const sf::Color color_) : map(map_), criteria(color_) {}
		sf::Vector2f getPosition(){ return D; }
		void setPosition(sf::Vector2f _newPos){
			A = sf::Vector2f(_newPos.x - 1, _newPos.y - 1);
			B = sf::Vector2f(_newPos.x, _newPos.y - 1);
			C = sf::Vector2f(_newPos.x - 1, _newPos.y);
			D = _newPos;
			AValue = map.getPixel(_newPos.x - 1, _newPos.y - 1) == criteria;
			BValue = map.getPixel(_newPos.x, _newPos.y - 1) == criteria;
			CValue = map.getPixel(_newPos.x - 1, _newPos.y) == criteria;
			DValue = map.getPixel(_newPos.x, _newPos.y) == criteria;
		}
		sf::Vector2f nextMove()
		{
			if (AValue && BValue && !CValue && !DValue) setPosition(sf::Vector2f(getPosition().x + 1, getPosition().y));
			else if (!AValue && !BValue && CValue && DValue) setPosition(sf::Vector2f(getPosition().x - 1, getPosition().y));
			else if (!AValue && BValue && !CValue && !DValue) setPosition(sf::Vector2f(getPosition().x + 1, getPosition().y));
			else if (AValue && BValue && CValue && !DValue) setPosition(sf::Vector2f(getPosition().x + 1, getPosition().y));
			else if (AValue && !BValue && !CValue && !DValue) setPosition(sf::Vector2f(getPosition().x, getPosition().y - 1));
			else if (AValue && !BValue && CValue && DValue) setPosition(sf::Vector2f(getPosition().x, getPosition().y - 1));
			else if (!AValue && BValue && !CValue && DValue) setPosition(sf::Vector2f(getPosition().x, getPosition().y + 1));
			else if (AValue && !BValue && CValue && !DValue) setPosition(sf::Vector2f(getPosition().x, getPosition().y - 1));
			else if (!AValue && BValue && CValue && DValue) setPosition(sf::Vector2f(getPosition().x - 1, getPosition().y));
			else if (!AValue && !BValue && !CValue && DValue) setPosition(sf::Vector2f(getPosition().x, getPosition().y + 1));
			else if (!AValue && !BValue && CValue && !DValue) setPosition(sf::Vector2f(getPosition().x - 1, getPosition().y));
			else if (AValue && BValue && !CValue && DValue) setPosition(sf::Vector2f(getPosition().x, getPosition().y + 1));
			else if (AValue && !BValue && !CValue && DValue) setPosition(sf::Vector2f(getPosition().x, getPosition().y - 1));
			else if (!AValue && BValue && CValue && !DValue) setPosition(sf::Vector2f(getPosition().x - 1, getPosition().y));
			return getPosition();
		}
	private:
		const sf::Image& map;
		sf::Vector2f A, B, C, D;
		bool AValue, BValue, CValue, DValue;
		const sf::Color criteria;
	};
	const sf::Image provinces = _map;
	std::vector<std::vector<sf::Vector2f>> returnValue;
	while (1)
	{
		sf::Vector2f start = findStartingPixel(_map, _color);
		if (start == sf::Vector2f(-1, -1))break;
		std::vector<sf::Vector2f> points;
		cell cell(_map, _color);
		cell.setPosition(start);
		points.push_back(start);
		points.push_back(cell.nextMove());
		while (cell.nextMove() != start)
		{
			points.push_back(cell.getPosition());
			if (points.size() > 5000)
			{
				std::cout << ("Error: max point count exceeded: (" + std::to_string(cell.getPosition().x) + ", " + std::to_string(cell.getPosition().y) + ')' + '\n');
				points.clear();
				break;
			}
		}
		utl::floodFillColorChange(_map, start.x, start.y, _color, sf::Color(0, 0, 0, 0));
		if (points.empty()) continue;
		else returnValue.push_back(points);
	}
	_map = provinces;
	return returnValue;
}

const std::vector<sf::Vector2f> utl::simplifyShape(const std::vector<sf::Vector2f>& points)
{
	std::vector<sf::Vector2f> returnValue;

	for (std::vector<sf::Vector2f>::const_iterator it = points.begin(), end = points.end(); it != end; ++it)
	{
		const sf::Vector2f A = it != points.begin() ? *(it - 1) : points.at(points.size() - 1), B = *it, C = it + 1 != points.end() ? *(it + 1) : *points.begin();
		if (!threePointsMakeALine(A, B, C))
		{
			returnValue.push_back(B);
		}
	}

	return returnValue;
}

const sf::VertexArray utl::tesselateShape(const std::vector<sf::Vector2f>& shape)
{
	if (shape.size() > 2)
	{
		std::vector<std::vector<sf::Vector2f>> shapes;
		shapes.push_back(shape);
		sf::VertexArray triangles(sf::PrimitiveType::Triangles, (shape.size() - 2) * 3);
		unsigned int vertexIndex = 0;

		while (!shapes.empty())
		{
			if (shapes.back().size() == 3)
			{
				triangles[vertexIndex].position = sf::Vector2f(shapes.back().at(0).x, shapes.back().at(0).y);
				triangles[vertexIndex + 1].position = sf::Vector2f(shapes.back().at(1).x, shapes.back().at(1).y);
				triangles[vertexIndex + 2].position = sf::Vector2f(shapes.back().at(2).x, shapes.back().at(2).y);
				vertexIndex += 3;
				shapes.pop_back();
				continue;
			}
			else if (shapes.back().size() < 3)
			{
				shapes.pop_back();
				continue;
			}

			sf::Vector2f A, B, C;
			int j = 0, k = 0, l = 0, q = 0;
			A.x = 1 << 15;
			for (int i = 0; i != shapes.back().size(); ++i)
			{
				if (shapes.back().at(i).x < A.x)
				{
					A.x = shapes.back().at(i).x;
					A.y = shapes.back().at(i).y;
					B.x = shapes.back().at(i == 0 ? shapes.back().size() - 1 : i - 1).x;
					B.y = shapes.back().at(i == 0 ? shapes.back().size() - 1 : i - 1).y;
					C.x = shapes.back().at(i == shapes.back().size() - 1 ? 0 : i + 1).x;
					C.y = shapes.back().at(i == shapes.back().size() - 1 ? 0 : i + 1).y;
					j = i == 0 ? shapes.back().size() - 1 : i - 1;
					k = i == shapes.back().size() - 1 ? 0 : i + 1;
					l = i;
				}
			}
			bool pointWasInside = false;
			for (unsigned int i = 0; i < shapes.back().size(); i++)
			{
				float alpha = (((B.y - C.y)*(shapes.back().at(i).x - C.x) + (C.x - B.x)*(shapes.back().at(i).y - C.y)) / ((B.y - C.y)*(A.x - C.x) + (C.x - B.x)*(A.y - C.y)));
				float beta = (((C.y - A.y)*(shapes.back().at(i).x - C.x) + (A.x - C.x)*(shapes.back().at(i).y - C.y)) / ((B.y - C.y)*(A.x - C.x) + (C.x - B.x)*(A.y - C.y)));
				float gamma = 1.00 - alpha - beta;
				if (alpha >= 0.00 && beta >= 0.00 && gamma >= 0.00 && sf::Vector2f(shapes.back().at(i)) != C && sf::Vector2f(shapes.back().at(i)) != B && sf::Vector2f(shapes.back().at(i)) != A)
				{
					pointWasInside = true;
					C = sf::Vector2f(shapes.back().at(i));
					q = i;
					i = 0;
				}
			}
			int i = 0;
			std::vector<sf::Vector2f> triangle;
			triangle.reserve(3);
			triangle.push_back(A);
			triangle.push_back(B);
			triangle.push_back(C);
			triangles[vertexIndex].position = sf::Vector2f(triangle.at(0).x, triangle.at(0).y);
			triangles[vertexIndex + 1].position = sf::Vector2f(triangle.at(1).x, triangle.at(1).y);
			triangles[vertexIndex + 2].position = sf::Vector2f(triangle.at(2).x, triangle.at(2).y);
			vertexIndex += 3;
			if (!pointWasInside)
			{
				std::vector<sf::Vector2f> _shape;
				_shape.reserve(shapes.back().size() - 1);
				i = j;
				while (1)
				{
					if (i < 0)
					{
						i = shapes.back().size() - 1;
					}
					_shape.push_back(shapes.back().at(i));
					if (i == k)
					{
						break;
					}
					i--;
				}
				shapes.pop_back();
				shapes.push_back(_shape);
			}
			else
			{
				std::vector<sf::Vector2f> _shape, shape_;
				i = l;
				std::vector<sf::Vector2f> pointsOfFirstShape;
				while (1)
				{
					pointsOfFirstShape.push_back(shapes.back().at(i));
					if (i == q)
					{
						break;
					}
					++i;
					i > shapes.back().size() - 1 ? i = 0 : i = i;
				}
				_shape.reserve(pointsOfFirstShape.size());
				for (std::vector<sf::Vector2f>::iterator it1 = pointsOfFirstShape.begin(); it1 != pointsOfFirstShape.end(); ++it1)
				{
					_shape.push_back(*it1);
				}
				shape_.reserve(shapes.back().size() - pointsOfFirstShape.size() + 1);
				pointsOfFirstShape.clear();
				while (1)
				{
					shape_.push_back(shapes.back().at(i));
					if (i == j)
					{
						break;
					}
					++i;
					i > shapes.back().size() - 1 ? i = 0 : i = i;
				}
				shapes.pop_back();
				shapes.push_back(_shape);
				shapes.push_back(shape_);
			}
		}
		return triangles;
	}
	return sf::VertexArray();
}

const sf::VertexArray utl::pointVectorToVertexArray(const std::vector<std::vector<sf::Vector2f>>& shapes)
{
	sf::VertexArray _RegionVArray(sf::PrimitiveType::Triangles);

	for (std::vector<std::vector<sf::Vector2f>>::const_iterator it = shapes.begin(), end = shapes.end(); it != end; ++it)
	{
		sf::VertexArray array = utl::tesselateShape(*it);
		for (unsigned long long i = 0; i < array.getVertexCount(); i++)
		{
			_RegionVArray.append(array[i].position);
		}
	}
	return _RegionVArray;
}

void utl::transferTriangles(const sf::VertexArray& source, sf::VertexArray& target)
{
	for (unsigned int i = 0; i < source.getVertexCount(); i++)
	{
		target.append(source[i]);
	}
}

const sf::Vector2f utl::leftmostPoint(const std::vector<sf::Vector2f>& points)
{
	sf::Vector2f min(1 << 14, 1 << 14);
	for (std::vector<sf::Vector2f>::const_iterator it = points.begin(), end = points.end(); it != end; ++it)
	{
		if (it->x < min.x) min = *it;
	}
	return min;
}

const utl::position utl::pointPositionRelativeToLine(const sf::Vector2f& A, const sf::Vector2f& B, const sf::Vector2f& C)
{
	return (B.x - A.x)*(C.y - A.y) - (B.y - A.y)*(C.x - A.x) < 0.0f ? LEFT : ((B.x - A.x)*(C.y - A.y) - (B.y - A.y)*(C.x - A.x) > 0.0f ? RIGHT : COLLINEAR);
}

const bool utl::existsAPointToTheLeft(const sf::Vector2f& A, const sf::Vector2f& B, const std::vector<sf::Vector2f>& points)
{
	for (std::vector<sf::Vector2f>::const_iterator it1 = points.begin(); it1 != points.end(); ++it1)
	{
		if (pointPositionRelativeToLine(A, B, *it1) == RIGHT)
		{
			return false;
		}
	}
	return true;
}

const float utl::distanceToPointSquared(const sf::Vector2f& A, const sf::Vector2f& B)
{
	return (A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y);
}

const std::vector<sf::Vector2f> utl::convexHull(const std::vector<sf::Vector2f>& _shape)
{
	std::vector<sf::Vector2f> returnValue;
	returnValue.push_back(leftmostPoint(_shape));
	for (std::vector<sf::Vector2f>::const_iterator it = _shape.begin(); it != _shape.end(); ++it)
	{
		bool allPointWereToTheLeft = true;
		for (std::vector<sf::Vector2f>::const_iterator it1 = _shape.begin(); it1 != _shape.end(); ++it1)
		{
			if (pointPositionRelativeToLine(returnValue.back(), *it, *it1) == RIGHT)
			{
				allPointWereToTheLeft = false;
				break;
			}
			else if (pointPositionRelativeToLine(returnValue.back(), *it, *it1) == COLLINEAR && existsAPointToTheLeft(returnValue.back(), *it1, _shape) && distanceToPointSquared(returnValue.back(), *it1) > distanceToPointSquared(returnValue.back(), *it))
			{
				it = it1;
				it1 = _shape.begin();
			}
		}
		if (allPointWereToTheLeft)
		{
			if (*it == *returnValue.begin()) break;
			returnValue.push_back(*it);
			it = _shape.begin();
		}
	}
	return returnValue;
}

const bool utl::haveMoreThanOneCommonPoint(const std::vector<sf::Vector2f>& shape, const std::vector<std::vector<sf::Vector2f>>& shapes)
{
	for (std::vector<std::vector<sf::Vector2f>>::const_iterator it = shapes.begin(), end = shapes.end(); it != end; ++it)
	{
		bool foundPoint = false;
		for (std::vector<sf::Vector2f>::const_iterator it1 = it->begin(), end1 = it->end(); it1 != end1; ++it1)
		{
			for (std::vector<sf::Vector2f>::const_iterator it2 = shape.begin(), end2 = shape.end(); it2 != end2; ++it2)
			{
				if (*it1 == *it2)
				{
					if (foundPoint) return true;
					else foundPoint = true;
				}
			}
		}
	}
	return false;
}

const bool utl::isPointInsideTriangle(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3, const sf::Vector2f& p)
{
	float alpha = ((p2.y - p3.y)*(p.x - p3.x) + (p3.x - p2.x)*(p.y - p3.y)) /
		((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));
	float beta = ((p3.y - p1.y)*(p.x - p3.x) + (p1.x - p3.x)*(p.y - p3.y)) /
		((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));
	float gamma = 1.0f - alpha - beta;

	return (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f);
}

template <class Element> const bool utl::arrayContainsElement(const std::vector<Element>& _array, const Element& _element)
{
	for (std::vector<Element>::const_iterator it = _array.begin(), end = _array.end(); it != end; ++it)
	{
		if (*it == _element) return true;
	}
	return false;
}

const std::vector<sf::Vector2f> utl::concaveHull(const std::vector<sf::Vector2f>& points)
{
	std::vector<sf::Vector2f> returnValue = points;

	sf::Vector2f min;
	for (auto a : returnValue)
	{
		if (a.x < min.x) min = a;
		if (a.x == min.x && min.y > min.y) min = a;
	}

	sort(returnValue.begin(), returnValue.end(),
		[min](sf::Vector2f a, sf::Vector2f b)
		{
		return atan2(min.x - a.x, min.y - a.y) < atan2(min.x - b.x, min.y - b.y);
		});

	auto newEnd = unique(returnValue.begin(), returnValue.end(),
		[](sf::Vector2f a, sf::Vector2f b)
		{
		return (a.x == b.x ? (a.y == b.y) : (a.x == b.y)); 
		});

	returnValue.erase(newEnd, returnValue.end());

	return returnValue;
}