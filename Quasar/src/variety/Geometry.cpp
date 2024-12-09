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

Logger& operator<<(Logger& log, const ClippingRect& cr)
{
    return log << "[ x=" << cr.x << ", y=" << cr.y << ", w=" << cr.screen_w << ", h=" << cr.screen_h << " ]";
}

Logger& operator<<(Logger& log, const Bounds& b)
{
    return log << "<" << b.x1 << ", " << b.x2 << ", " << b.y1 << ", " << b.y2 << ">";
}

Logger& operator<<(Logger& log, const IntBounds& ib)
{
    return log << "<" << ib.x1 << ", " << ib.x2 << ", " << ib.y1 << ", " << ib.y2 << ">";
}

Logger& operator<<(Logger& log, const IntRect& ir)
{
    return log << "[ x=" << ir.x << ", y=" << ir.y << ", w=" << ir.w << ", h=" << ir.h << " ]";
}
