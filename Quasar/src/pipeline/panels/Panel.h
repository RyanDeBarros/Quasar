#pragma once

#include <vector>
#include <memory>

#include "variety/Geometry.h"

struct PanelGroup;

struct Panel
{
	PanelGroup* pgroup = nullptr;
	bool visible = true;
	FlatTransform view{};
	IntBounds bounds;

	Panel() = default;
	Panel(const Panel&) = delete;
	Panel(Panel&&) noexcept = delete;
	virtual ~Panel() = default;

	virtual void draw() = 0;
	virtual void send_view() = 0;
	void render();
	glm::mat3 vp_matrix() const;

	bool cursor_in_clipping() const;
	float get_app_width() const;
	float get_app_height() const;
	glm::vec2 get_app_cursor_pos() const;
};

struct PanelGroup
{
	std::vector<std::unique_ptr<Panel>> panels;
	glm::mat3 projection{};
private:
	friend Panel;
	Scale app_scale;
public:

	PanelGroup() = default;
	PanelGroup(const PanelGroup&) = delete;
	PanelGroup(PanelGroup&&) noexcept = delete;

	void sync_panels();
	void render();
	void set_projection();

	void set_app_scale(Scale sc);
	Scale get_app_scale() const;
};
