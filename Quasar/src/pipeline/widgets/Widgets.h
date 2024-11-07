#pragma once

#include "variety/Geometry.h"
#include "variety/Utils.h"
#include "../render/UnitRenderable.h"

struct WidgetPlacement
{
	FlatTransform transform{};
	glm::vec2 pivot{ 0.5f, 0.5f };

	WidgetPlacement relative_to(const FlatTransform& parent) const { return { transform.relative_to(parent), pivot }; }

	float clamp_x(float x) const { return std::clamp(x, left(), right()); }
	float clamp_y(float y) const { return std::clamp(y, bottom(), top()); }
	Position clamp_point(Position pos) const { return { clamp_x(pos.x), clamp_y(pos.y) }; }
	Position clamp_point_in_ellipse(Position pos) const
	{
		Position cp = center_point();
		Position rel = pos - cp;
		float axis_x = 0.5f * transform.scale.x;
		float axis_y = 0.5f * transform.scale.y;
		// x^2 / axis_x^2 + y^2 / axis_y^2
		float elliptical_radius = (rel.x / axis_x) * (rel.x / axis_x) + (rel.y / axis_y) * (rel.y / axis_y);
		if (elliptical_radius <= 1.0f)
			return pos;
		return cp + rel * glm::inversesqrt(elliptical_radius);
	}

	bool contains_x(float x) const { return on_interval(x, left(), right()); }
	bool contains_y(float y) const { return on_interval(y, bottom(), top()); }
	bool contains_point(Position pos) const { return contains_x(pos.x) && contains_y(pos.y); }

	float center_x() const { return transform.position.x + (0.5f - pivot.x) * transform.scale.x; }
	float center_y() const { return transform.position.y + (0.5f - pivot.y) * transform.scale.y; }
	Position center_point() const { return { center_x(), center_y() }; }

	float left() const { return transform.position.x - pivot.x * transform.scale.x; }
	float right() const { return transform.position.x + (1.0f - pivot.x) * transform.scale.x; }
	float bottom() const { return transform.position.y - pivot.y * transform.scale.y; }
	float top() const { return transform.position.y + (1.0f - pivot.y) * transform.scale.y; }

	float normalize_x(float x) const { return (x - transform.position.x + pivot.x * transform.scale.x) / transform.scale.x; }
	float normalize_y(float y) const { return (y - transform.position.y + pivot.y * transform.scale.y) / transform.scale.y; }
	Position normalize(Position pos) const { return { normalize_x(pos.x), normalize_y(pos.y) }; }

	float interp_x(float t) const { return left() + t * transform.scale.x; }
	float interp_y(float t) const { return bottom() + t * transform.scale.y; }

	glm::mat3 matrix() const { return FlatTransform{ center_point(), transform.scale }.matrix(); }
	WidgetPlacement inverse() const { return { transform.inverse(), { 1.0f - pivot.x, 1.0f - pivot.y } }; }
};

inline float wp_left(const glm::mat3& global) { return global[2][0] - 0.5f * global[0][0]; }
inline float wp_right(const glm::mat3& global) { return global[2][0] + 0.5f * global[0][0]; }
inline float wp_bottom(const glm::mat3& global) { return global[2][1] - 0.5f * global[1][1]; }
inline float wp_top(const glm::mat3& global) { return global[2][1] + 0.5f * global[1][1]; }

struct Widget
{
	Widget* parent = nullptr;
	WidgetPlacement self;
	std::vector<Widget*> children; // LATER use shared_ptr?

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

	glm::mat3 global_matrix() const { if (parent) return parent->global_matrix() * self.matrix(); else return self.matrix(); }
	glm::mat3 global_matrix_inverse() const { if (parent) return self.inverse().matrix() * parent->global_matrix_inverse(); else return self.inverse().matrix(); }
	Position global_of(Position local) const { glm::vec3 g = global_matrix() * glm::vec3(local, 1.0f); return { g.x, g.y }; }
	Position local_of(Position global) const { glm::vec3 l = global_matrix_inverse() * glm::vec3(global, 1.0f); return { l.x, l.y }; }
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
		if (parent)
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
