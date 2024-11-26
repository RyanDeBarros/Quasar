#include "Geometry.h"

#include "Utils.h"

Logger& operator<<(Logger& log, const Position& pos)
{
    return log << "P(" << pos.x << ", " << pos.y << ")";
}

Logger& operator<<(Logger& log, const IPosition& pos)
{
    return log << "P(" << pos.x << ", " << pos.y << ")";
}

Logger& operator<<(Logger& log, const Scale& sc)
{
    return log << "S(" << sc.x << ", " << sc.y << ")";
}

Logger& operator<<(Logger& log, const FlatTransform& tr)
{
    return log << "[" << tr.position << " | " << tr.scale << "]";
}

DiscreteLineInterpolator::DiscreteLineInterpolator(IPosition start, IPosition finish)
    : start(start), finish(finish), delta(finish - start), length(size_t(1) + std::max(std::abs(delta.x), std::abs(delta.y))) {}

IPosition DiscreteLineInterpolator::at(int i) const
{
    IPosition pos;
    at(i, pos);
    return pos;
}

void DiscreteLineInterpolator::at(int i, IPosition& pos) const
{
    at(i, pos.x, pos.y);
}

void DiscreteLineInterpolator::at(int i, int& x, int& y) const
{
    float fraction = length == 1 ? 0 : (float)i / (length - 1);
    x = start.x + (std::signbit(delta.x) ? -1 : 1) * roundi_down_on_half(std::abs(delta.x) * fraction);
    y = start.y + (std::signbit(delta.y) ? -1 : 1) * roundi_down_on_half(std::abs(delta.y) * fraction);
}
