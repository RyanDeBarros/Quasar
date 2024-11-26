#include "Geometry.h"

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
    pos = start;
    float fraction = length == 1 ? 0 : (float)i / (length - 1);
    pos.x += int(delta.x * fraction);
    pos.y += int(delta.y * fraction);
}
