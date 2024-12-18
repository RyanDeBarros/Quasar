#pragma once

#include <unordered_set>

#include "../../widgets/Widget.h"
#include "edit/color/Color.h"

class SelectionMants : public W_UnitRenderable
{
	std::unordered_set<IPosition> points;
	Shader shader;
	int rows = 0, cols = 0;

public:
	RGBA color1 = RGBA::WHITE, color2 = RGBA::BLACK;
	float speed = 10.0f;

	SelectionMants();
	SelectionMants(const SelectionMants&) = delete;
	SelectionMants(SelectionMants&&) noexcept = delete;

	void set_size(int width, int height);

	bool add(IPosition pos);
	bool remove(IPosition pos);

private:
	void shader_add(IPosition pos);
	void shader_remove(IPosition pos);

public:
	IntBounds clear();
	const std::unordered_set<IPosition>& get_points() const { return points; }

	virtual void draw() override;
	void send_uniforms() const;

	void send_buffer(IntBounds bbox);

	void send_vp(const glm::mat3& vp) const;
	void send_time(float time) const;
	void send_screen_size(glm::ivec2 size) const;
	void flip_direction();

	unsigned int vertex_horizontal(int x, int y) const;
	unsigned int vertex_vertical(int x, int y) const;
};

inline SelectionMants& sm_wget(Widget& w, size_t i)
{
	return *w.get<SelectionMants>(i);
}

inline const SelectionMants& sm_wget(const Widget& w, size_t i)
{
	return *w.get<SelectionMants>(i);
}
