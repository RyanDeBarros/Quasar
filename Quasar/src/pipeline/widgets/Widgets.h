#pragma once

#include "variety/Geometry.h"
#include "variety/Utils.h"
#include "../render/UnitRenderable.h"

struct Widget
{
	// TODO parent should be a PlacementHolder*, and there should be a global() call chain for WidgetPlacement. In addition, add a WidgetPlacement self.
	Widget* parent = nullptr;
	WidgetPlacement self;
	std::vector<Widget*> children; // TODO use shared_ptr?

	Widget(size_t null_length = 0)
	{
		for (size_t i = 0; i < null_length; ++i)
			children.push_back(nullptr);
	}
	Widget(std::vector<Widget*>&& heap_children) : children(std::move(heap_children)) {}
	Widget(const Widget&) = delete;
	Widget(Widget&&) noexcept = delete;
	virtual ~Widget() { for (auto ptr : children) delete ptr; }

	template<std::derived_from<Widget> T>
	T* get(size_t i) { return dynamic_cast<T*>(children[i]); }
	template<std::derived_from<Widget> T>
	const T* get(size_t i) const { return dynamic_cast<T*>(children[i]); }
	WidgetPlacement& wp_at(size_t i) { return children[i]->self; }
	const WidgetPlacement& wp_at(size_t i) const { return children[i]->self; }
};

inline void detach_widget(Widget* parent, Widget* child)
{
	if (parent && child)
	{
		auto iter = std::find(parent->children.begin(), parent->children.end(), child);
		if (iter != parent->children.end())
			parent->children.erase(iter);
		child->parent = nullptr;
	}
}

inline void attach_widget(Widget* parent, Widget* child)
{
	if (child)
	{
		detach_widget(child->parent, child);
		if (parent && child)
		{
			parent->children.push_back(child);
			child->parent = parent;
		}
	}
}

inline void assign_widget(Widget* parent, size_t pos, Widget* child)
{
	if (child)
	{
		detach_widget(child->parent, child);
		if (parent)
		{
			parent->children[pos] = child;
			child->parent = parent;
		}
	}
	else if (parent)
		parent->children[pos] = nullptr;

}

struct WP_UnitRenderable : public Widget
{
	std::unique_ptr<UnitRenderable> ur;

	WP_UnitRenderable(Shader* shader, unsigned char num_vertices = 4) : ur(std::make_unique<UnitRenderable>(shader, num_vertices)) {}
};

inline UnitRenderable& ur_wget(Widget& w, size_t i)
{
	return *w.get<WP_UnitRenderable>(i)->ur;
}

inline const UnitRenderable& ur_wget(const Widget& w, size_t i)
{
	return *w.get<WP_UnitRenderable>(i)->ur;
}

struct WP_UnitMultiRenderable : public Widget
{
	std::unique_ptr<UnitMultiRenderable> umr;

	WP_UnitMultiRenderable(Shader* shader, unsigned short num_units, unsigned char unit_num_vertices = 4) : umr(std::make_unique<UnitMultiRenderable>(shader, num_units, unit_num_vertices)) {}
};

inline UnitMultiRenderable& umr_wget(Widget& w, size_t i)
{
	return *w.get<WP_UnitMultiRenderable>(i)->umr;
}

inline const UnitMultiRenderable& umr_wget(const Widget& w, size_t i)
{
	return *w.get<WP_UnitMultiRenderable>(i)->umr;
}

struct WP_IndexedRenderable : public Widget
{
	std::unique_ptr<IndexedRenderable> ir;

	WP_IndexedRenderable(Shader* shader) : ir(std::make_unique<IndexedRenderable>(shader)) {}
};

inline IndexedRenderable& ir_wget(Widget& w, size_t i)
{
	return *w.get<WP_IndexedRenderable>(i)->ir;
}

inline const IndexedRenderable& ir_wget(const Widget& w, size_t i)
{
	return *w.get<WP_IndexedRenderable>(i)->ir;
}
