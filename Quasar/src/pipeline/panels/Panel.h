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
	IntBounds bounds{};

	Panel() = default;
	Panel(const Panel&) = delete;
	Panel(Panel&&) noexcept = delete;
	virtual ~Panel() = default;

	virtual void draw() = 0;
	void render();
	glm::mat3 vp_matrix() const;
	glm::mat3 vp_matrix_inverse() const;
	virtual void _send_view() = 0;
	void send_view();

	bool cursor_in_clipping() const;
	glm::vec2 get_app_size() const;
	float get_app_width() const;
	float get_app_height() const;
	glm::vec2 get_app_cursor_pos() const;

	Position to_view_coordinates(Position screen_coordinates) const;
	Position to_world_coordinates(Position screen_coordinates) const;
	Position to_screen_coordinates(Position world_coordinates) const;
	Scale to_view_size(Scale screen_size) const;
	Scale to_world_size(Scale screen_size) const;
	Scale to_screen_size(Scale world_size) const;

	virtual Scale minimum_screen_display() const { return {}; }
};

struct PanelGroup
{
	std::vector<std::unique_ptr<Panel>> panels;
	glm::mat3 projection{};

	PanelGroup() = default;
	PanelGroup(const PanelGroup&) = delete;
	PanelGroup(PanelGroup&&) noexcept = delete;

	void sync_panels();
	void render();
	void set_projection();
};
