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
