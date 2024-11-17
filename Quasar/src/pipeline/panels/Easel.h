#pragma once

#include "Panel.h"
#include "user/Platform.h"
#include "../render/FlatSprite.h"
#include "../render/Shader.h"
#include "../widgets/Widget.h"

struct Gridlines
{
	unsigned short width = 0, height = 0;
	GLuint vao = 0, vb = 0;
	Shader shader;
	GLfloat* varr = nullptr;
	glm::vec2 line_spacing = { 1.0f, 1.0f };
	float line_width = 1.0f; // SETTINGS
	float self_intersection_threshold = 1.0f; // SETTINGS

	GLint* arrays_firsts = nullptr;
	GLsizei* arrays_counts = nullptr;

private:
	mutable bool _visible = true;
public:

	Gridlines();
	Gridlines(const Gridlines&) = delete;
	Gridlines(Gridlines&&) noexcept = delete;
	~Gridlines();

	void resize_grid(Scale scale);
	void update_scale(Scale scale) const;
	void draw() const;

	unsigned short num_cols() const;
	unsigned short num_rows() const;
	GLsizei num_quads() const { return num_rows() + num_cols(); }
	GLsizei num_vertices() const { return num_quads() * 4; }

	void set_color(ColorFrame color) const;
};

// TODO checkerboard should use separate renderable
struct Canvas : public Widget
{
	FlatSprite sprite;
	FlatSprite checkerboard;
	RGBA checker1, checker2;

	Gridlines minor_gridlines;
	Gridlines major_gridlines;
private:
	glm::vec2 checker_size_inv = glm::vec2(1.0f / 16.0f);
public:
	glm::ivec2 get_checker_size() const { return { roundf(1.0f / checker_size_inv.x), roundf(1.0f / checker_size_inv.y) }; }
	void set_checker_size(glm::ivec2 checker_size);

	Canvas(Shader* sprite_shader);
	Canvas(const Canvas&) = delete;
	Canvas(Canvas&&) noexcept = delete;

	void set_image(const std::shared_ptr<Image>& img);
	void set_image(std::shared_ptr<Image>&& img);

	void sync_transform();

	void create_checkerboard_image();
	void sync_checkerboard_colors() const;
	void sync_checkerboard_texture() const;
	void set_checkerboard_uv_size(float width, float height) const;
};

struct Easel : public Panel
{
	Shader bkg_shader;
	Shader sprite_shader;
	Widget widget;
	Canvas canvas;

	bool canvas_visible = false;

private:
	bool minor_gridlines_visible = false;
	bool _buffer_minor_gridlines_send_flat_transform = false;
	bool _buffer_minor_gridlines_sync_with_image = false;
	bool major_gridlines_visible = false;
	bool _buffer_major_gridlines_send_flat_transform = false;
	bool _buffer_major_gridlines_sync_with_image = false;

public:
	Easel();
	Easel(const Easel&) = delete;
	Easel(Easel&&) noexcept = delete;

	void initialize_widget();

	virtual void draw() override;

	void send_gridlines_vao(const Gridlines& gridlines) const;
	
	virtual void _send_view() override;
	void sync_widget();
	
	void sync_canvas_transform();
	
	void resize_gridlines(Gridlines& gridlines) const;
	void update_gridlines_scale(const Gridlines& gridlines) const;
	void gridlines_send_flat_transform(Gridlines& gridlines) const;
	void gridlines_sync_with_image(Gridlines& gridlines) const;
	
	void set_canvas_image(const std::shared_ptr<Image>& image);
	void set_canvas_image(std::shared_ptr<Image>&& image);
	void update_canvas_image() { set_canvas_image(canvas.sprite.image); sync_canvas_transform(); }
	Image* canvas_image() const { return canvas.sprite.image.get(); }
	std::shared_ptr<Image> canvas_image_ref() const { return canvas.sprite.image; }

	bool minor_gridlines_are_visible() const { return minor_gridlines_visible; }
	void set_minor_gridlines_visibility(bool visible);
	bool major_gridlines_are_visible() const { return major_gridlines_visible; }
	void set_major_gridlines_visibility(bool visible);

	struct
	{
		Position initial_cursor_pos{};
		Position initial_canvas_pos{};
		bool panning = false;
	private:
		friend Easel;
		WindowHandle wh;
	} panning_info;
	struct
	{
		// SETTINGS (only some of them?)
		constexpr static float initial = 0.5f;
		constexpr static float in_min = 0.01f;
		constexpr static float in_max = 100.0f;
		constexpr static float factor = 1.5f;
		constexpr static float factor_shift = 1.05f;
		float zoom = initial;
	} zoom_info;

	void begin_panning();
	void end_panning();
	void cancel_panning();
	void update_panning();
	void zoom_by(float zoom);
	void reset_camera();

	enum : size_t
	{
		BACKGROUND,
		_W_COUNT
	};
};
