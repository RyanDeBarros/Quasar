#pragma once

#include <unordered_set>

#include "../widgets/Widget.h"

class SelectionMants : public W_UnitRenderable
{
	std::unordered_set<IPosition> points;
	Shader shader;
	int rows = 0, cols = 0;

public:
	SelectionMants();
	SelectionMants(const SelectionMants&) = delete;
	SelectionMants(SelectionMants&&) noexcept = delete;

	void set_size(int width, int height);

	void add(IPosition pos);
	void remove(IPosition pos);

	virtual void draw() override;

	void send_vp(const glm::mat3& vp) const;
	void send_time(float time) const;
	void send_screen_size(glm::ivec2 size) const;

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
