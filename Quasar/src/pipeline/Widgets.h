#pragma once

#include "variety/Geometry.h"
#include "variety/Utils.h"
#include "UnitRenderable.h"

struct Widget
{
	FlatTransform parent;
	std::vector<PolyHolder<WidgetPlacement>*> hobjs;

	Widget(size_t null_length = 0)
	{
		for (size_t i = 0; i < null_length; ++i)
			hobjs.push_back(nullptr);
	}
	Widget(std::vector<PolyHolder<WidgetPlacement>*>&& heap_hobjs) : hobjs(std::move(heap_hobjs)) {}
	Widget(const Widget&) = delete;
	Widget(Widget&&) noexcept = delete;
	~Widget() { for (auto ptr : hobjs) delete ptr; }

	template<typename T>
	T* get(size_t i) { return dynamic_cast<T*>(hobjs[i]); }
	template<typename T>
	const T* get(size_t i) const { return dynamic_cast<T*>(hobjs[i]); }
	WidgetPlacement& wp_at(size_t i) { return hobjs[i]->held; }
	const WidgetPlacement& wp_at(size_t i) const { return hobjs[i]->held; }
};

template<typename Held>
struct H_UnitRenderable : public PolyHolder<Held>
{
	std::unique_ptr<UnitRenderable> ur;

	H_UnitRenderable(Shader& shader, unsigned char num_vertices = 4) : ur(std::make_unique<UnitRenderable>(shader, num_vertices)) {}
};

typedef H_UnitRenderable<FlatTransform> FT_UnitRenderable;
typedef H_UnitRenderable<WidgetPlacement> WP_UnitRenderable;

inline UnitRenderable& ur_wget(Widget& w, size_t i)
{
	return *w.get<WP_UnitRenderable>(i)->ur;
}

inline const UnitRenderable& ur_wget(const Widget& w, size_t i)
{
	return *w.get<WP_UnitRenderable>(i)->ur;
}

template<typename Held>
struct H_UnitMultiRenderable : public PolyHolder<Held>
{
	std::unique_ptr<UnitMultiRenderable> umr;

	H_UnitMultiRenderable(Shader& shader, unsigned short num_units, unsigned char unit_num_vertices = 4) : umr(std::make_unique<UnitMultiRenderable>(shader, num_units, unit_num_vertices)) {}
};

typedef H_UnitMultiRenderable<FlatTransform> FT_UnitMultiRenderable;
typedef H_UnitMultiRenderable<WidgetPlacement> WP_UnitMultiRenderable;

inline UnitMultiRenderable& umr_wget(Widget& w, size_t i)
{
	return *w.get<WP_UnitMultiRenderable>(i)->umr;
}

inline const UnitMultiRenderable& umr_wget(const Widget& w, size_t i)
{
	return *w.get<WP_UnitMultiRenderable>(i)->umr;
}
