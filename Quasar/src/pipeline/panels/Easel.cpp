#include "Easel.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"
#include "user/Machine.h"
#include "../render/Uniforms.h"

Gridlines::Gridlines()
	: shader(FileSystem::shader_path("gridlines.vert"), FileSystem::shader_path("gridlines.frag"))
{
	initialize_dynamic_vao(vao, vb, 0, shader.stride, varr, shader.attributes);
}

Gridlines::~Gridlines()
{
	delete_vao_buffers(vao, vb);
	delete[] varr;
	delete[] arrays_firsts;
	delete[] arrays_counts;
}

void Gridlines::resize_grid(Scale scale)
{
	delete[] varr;
	varr = new GLfloat[num_vertices() * shader.stride];
	update_scale(scale);
#pragma warning(push)
#pragma warning(disable : 6386)
	delete[] arrays_firsts;
	delete[] arrays_counts;
	arrays_firsts = new GLint[num_quads()];
	arrays_counts = new GLsizei[num_quads()];
	for (unsigned short i = 0; i < num_quads(); ++i)
	{
		arrays_firsts[i] = 4 * i;
		arrays_counts[i] = 4;
	}
#pragma warning(pop)
}

void Gridlines::update_scale(Scale scale) const
{
	if (!varr) return;
	GLfloat* setter = varr;

#pragma warning(push)
#pragma warning(disable : 6386)
	float lwx = 0.5f * line_width;
	if (scale.x > 1.0f)
		lwx /= scale.x;
	float lwy = 0.5f * line_width;
	if (scale.y > 1.0f)
		lwy /= scale.y;
	if (2.0f * lwx >= scale.x * line_spacing.x - self_intersection_threshold || 2.0f * lwy >= scale.y * line_spacing.y - self_intersection_threshold)
	{
		_visible = false;
		return;
	}
	else
		_visible = true;

	float x1 = -width * 0.5f - lwx;
	float y1 = -height * 0.5f - lwy;
	float x2 = -width * 0.5f + lwx;
	float y2 = height * 0.5f + lwy;

	for (unsigned short i = 0; i < num_cols() - 1; ++i)
	{
		float delta = i * line_spacing.x;
		setter[0] = x1 + delta;
		setter[1] = y1;
		setter += shader.stride;
		setter[0] = x2 + delta;
		setter[1] = y1;
		setter += shader.stride;
		setter[0] = x1 + delta;
		setter[1] = y2;
		setter += shader.stride;
		setter[0] = x2 + delta;
		setter[1] = y2;
		setter += shader.stride;
	}

	x1 = width * 0.5f - lwx;
	x2 = width * 0.5f + lwx;
	setter[0] = x1;
	setter[1] = y1;
	setter += shader.stride;
	setter[0] = x2;
	setter[1] = y1;
	setter += shader.stride;
	setter[0] = x1;
	setter[1] = y2;
	setter += shader.stride;
	setter[0] = x2;
	setter[1] = y2;
	setter += shader.stride;

	x1 = -width * 0.5f - lwx;
	y1 = -height * 0.5f - lwy;
	x2 = width * 0.5f + lwx;
	y2 = -height * 0.5f + lwy;
	for (unsigned short i = 0; i < num_rows() - 1; ++i)
	{
		float delta = i * line_spacing.y;
		setter[0] = x1;
		setter[1] = y1 + delta;
		setter += shader.stride;
		setter[0] = x1;
		setter[1] = y2 + delta;
		setter += shader.stride;
		setter[0] = x2;
		setter[1] = y1 + delta;
		setter += shader.stride;
		setter[0] = x2;
		setter[1] = y2 + delta;
		setter += shader.stride;
	}

	y1 = height * 0.5f - lwy;
	y2 = height * 0.5f + lwy;
	setter[0] = x1;
	setter[1] = y1;
	setter += shader.stride;
	setter[0] = x1;
	setter[1] = y2;
	setter += shader.stride;
	setter[0] = x2;
	setter[1] = y1;
	setter += shader.stride;
	setter[0] = x2;
	setter[1] = y2;
	setter += shader.stride;
#pragma warning(pop)
}

void Gridlines::draw() const
{
	if (_visible)
	{
		bind_shader(shader);
		bind_vao_buffers(vao, vb);
		QUASAR_GL(glMultiDrawArrays(GL_TRIANGLE_STRIP, arrays_firsts, arrays_counts, num_quads()));
	}
}

unsigned short Gridlines::num_cols() const
{
	return unsigned short(std::ceil(width / line_spacing.x)) + 1;
}

unsigned short Gridlines::num_rows() const
{
	return unsigned short(std::ceil(height / line_spacing.y)) + 1;
}

void Gridlines::set_color(ColorFrame color) const
{
	Uniforms::send_4(shader, "u_Color", color.rgba().as_vec(), 0, true);
}

void Canvas::create_checkerboard_image()
{
	Image* img = new Image();
	img->buf.width = 2;
	img->buf.height = 2;
	img->buf.chpp = 4;
	img->buf.pxnew();
	img->gen_texture();
	checkerboard.set_image(std::shared_ptr<Image>(img));
	set_checkerboard_uv_size(0, 0);
}

void Canvas::sync_checkerboard_colors() const
{
	if (checkerboard.image)
	{
		for (size_t i = 0; i < 2; ++i)
		{
			checkerboard.image->buf.pixels[0 + 12 * i] = checker1.get_pixel_r();
			checkerboard.image->buf.pixels[1 + 12 * i] = checker1.get_pixel_g();
			checkerboard.image->buf.pixels[2 + 12 * i] = checker1.get_pixel_b();
			checkerboard.image->buf.pixels[3 + 12 * i] = checker1.get_pixel_a();
		}
		for (size_t i = 0; i < 2; ++i)
		{
			checkerboard.image->buf.pixels[4 + 4 * i] = checker2.get_pixel_r();
			checkerboard.image->buf.pixels[5 + 4 * i] = checker2.get_pixel_g();
			checkerboard.image->buf.pixels[6 + 4 * i] = checker2.get_pixel_b();
			checkerboard.image->buf.pixels[7 + 4 * i] = checker2.get_pixel_a();
		}
	}
	sync_checkerboard_texture();
}

void Canvas::sync_checkerboard_texture() const
{
	if (checkerboard.image)
	{
		checkerboard.image->resend_texture();
		TextureParams tparams;
		tparams.wrap_s = TextureWrap::Repeat;
		tparams.wrap_t = TextureWrap::Repeat;
		checkerboard.image->update_texture_params(tparams);
	}
}

void Canvas::set_checkerboard_uv_size(float width, float height) const
{
	checkerboard.set_uvs(Bounds{ 0.0f, width, 0.0f, height });
}

void Canvas::set_checker_size(glm::ivec2 checker_size)
{
	checker_size_inv = { 1.0f / checker_size.x, 1.0f / checker_size.y };
	major_gridlines.line_spacing = checker_size;
}

Canvas::Canvas(Shader* sprite_shader)
	: sprite(sprite_shader), checkerboard(sprite_shader)
{
}

void Canvas::set_image(const std::shared_ptr<Image>& img)
{
	sprite.set_image(img);
	if (sprite.image)
	{
		set_checkerboard_uv_size(0.5f * sprite.image->buf.width * checker_size_inv.x, 0.5f * sprite.image->buf.height * checker_size_inv.y);
		checkerboard.set_modulation(ColorFrame());
	}
	else
		checkerboard.set_modulation(ColorFrame(0));
	checkerboard.ur->send_buffer();
}

void Canvas::set_image(std::shared_ptr<Image>&& img)
{
	sprite.set_image(std::move(img));
	if (sprite.image)
	{
		set_checkerboard_uv_size(0.5f * sprite.image->buf.width * checker_size_inv.x, 0.5f * sprite.image->buf.height * checker_size_inv.y);
		checkerboard.set_modulation(ColorFrame());
	}
	else
		checkerboard.set_modulation(ColorFrame(0));
	checkerboard.ur->send_buffer();
}

void Canvas::sync_transform()
{
	sprite.self.transform = self.transform;
	sprite.sync_transform();
	sprite.ur->send_buffer();
	// TODO flat sprite's should use global transforms, instead of directly assigning parent to them here
	checkerboard.self.transform = self.transform;
	if (sprite.image)
		checkerboard.self.transform.scale *= glm::vec2{ sprite.image->buf.width * 0.5f, sprite.image->buf.height * 0.5f };
	checkerboard.sync_transform();
	checkerboard.ur->send_buffer();
}

constexpr float CHECKERBOARD_TSLOT = 0.0f;
constexpr float CANVAS_SPRITE_TSLOT = 1.0f;

Easel::Easel()
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", std::to_string(GLC.max_texture_image_units) }}),
	bkg_shader(FileSystem::shader_path("color_square.vert"), FileSystem::shader_path("color_square.frag")), widget(_W_COUNT), canvas(&sprite_shader)
{
	initialize_widget();

	canvas.create_checkerboard_image();
	canvas.set_image(nullptr);

	canvas.checker1 = Machine.preferences.checker1;
	canvas.checker2 = Machine.preferences.checker2;
	canvas.set_checker_size(Machine.preferences.checker_size);
	canvas.sync_checkerboard_colors();

	canvas.checkerboard.set_texture_slot(CHECKERBOARD_TSLOT);
	canvas.sprite.set_texture_slot(CANVAS_SPRITE_TSLOT);
}

void Easel::initialize_widget()
{
	assign_widget(&widget, BACKGROUND, std::make_shared<W_UnitRenderable>(&bkg_shader));
	ur_wget(widget, BACKGROUND).set_attribute(1, glm::value_ptr(RGBA(HSV(0.5f, 0.15f, 0.15f).to_rgb(), 0.5f).as_vec()));
	ur_wget(widget, BACKGROUND).send_buffer();
}

void Easel::draw()
{
	ur_wget(widget, BACKGROUND).draw();
	// bind
	bind_shader(sprite_shader);
	// canvas
	if (canvas_visible)
	{
		// checkerboard
		bind_texture(canvas.checkerboard.image->tid, GLuint(CHECKERBOARD_TSLOT));
		canvas.checkerboard.draw();
		if (canvas.sprite.image)
		{
			bind_texture(canvas.sprite.image->tid, GLuint(CANVAS_SPRITE_TSLOT));
			canvas.sprite.draw();
		}
		// minor gridlines
		if (minor_gridlines_visible)
			canvas.minor_gridlines.draw();
		// major gridlines
		if (major_gridlines_visible)
			canvas.major_gridlines.draw();
	}
	// unbind
	unbind_vao_buffers();
	unbind_shader();
}

void Easel::send_gridlines_vao(const Gridlines& gridlines) const
{
	bind_vao_buffers(gridlines.vao, gridlines.vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, gridlines.num_vertices() * gridlines.shader.stride * sizeof(GLfloat), gridlines.varr, GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

void Easel::_send_view()
{
	glm::mat3 cameraVP = vp_matrix();
	Uniforms::send_matrix3(sprite_shader, "u_VP", cameraVP);
	Uniforms::send_matrix3(bkg_shader, "u_VP", cameraVP);
	Uniforms::send_matrix3(canvas.minor_gridlines.shader, "u_VP", cameraVP);
	Uniforms::send_matrix3(canvas.major_gridlines.shader, "u_VP", cameraVP);
	unbind_shader();
	sync_widget();
}

void Easel::sync_widget()
{
	widget.wp_at(BACKGROUND).transform.scale = get_app_size();
	WidgetPlacement wp = widget.wp_at(BACKGROUND).relative_to(widget.self.transform);
	UnitRenderable& bkg = ur_wget(widget, BACKGROUND);
	bkg.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ wp.left(), wp.bottom() }));
	bkg.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ wp.right(), wp.bottom() }));
	bkg.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ wp.left(), wp.top() }));
	bkg.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ wp.right(), wp.top() }));
	bkg.send_buffer();
}

void Easel::sync_canvas_transform()
{
	canvas.sync_transform();
	if (minor_gridlines_visible)
		gridlines_send_flat_transform(canvas.minor_gridlines);
	else
		_buffer_minor_gridlines_send_flat_transform = true;
	if (major_gridlines_visible)
		gridlines_send_flat_transform(canvas.major_gridlines);
	else
		_buffer_major_gridlines_send_flat_transform = true;
	unbind_shader();
}

void Easel::gridlines_send_flat_transform(Gridlines& gridlines) const
{
	Uniforms::send_4(gridlines.shader, "u_FlatTransform", canvas.self.transform.packed());
	update_gridlines_scale(gridlines);
}

void Easel::resize_gridlines(Gridlines& gridlines) const
{
	gridlines.resize_grid(canvas.self.transform.scale);
	send_gridlines_vao(gridlines);
}

void Easel::update_gridlines_scale(const Gridlines& gridlines) const
{
	gridlines.update_scale(canvas.self.transform.scale);
	send_gridlines_vao(gridlines);
}

void Easel::gridlines_sync_with_image(Gridlines& gridlines) const
{
	gridlines.width = canvas.sprite.image->buf.width;
	gridlines.height = canvas.sprite.image->buf.height;
	resize_gridlines(gridlines);
	send_gridlines_vao(gridlines);
}

void Easel::set_canvas_image(const std::shared_ptr<Image>& img)
{
	canvas.set_image(img);
	canvas_visible = true;
	if (minor_gridlines_visible)
		gridlines_sync_with_image(canvas.minor_gridlines);
	else
		_buffer_minor_gridlines_sync_with_image = true;
	if (major_gridlines_visible)
		gridlines_sync_with_image(canvas.major_gridlines);
	else
		_buffer_major_gridlines_sync_with_image = true;
}

void Easel::set_canvas_image(std::shared_ptr<Image>&& img)
{
	canvas.set_image(std::move(img));
	canvas_visible = true;
	if (minor_gridlines_visible)
		gridlines_sync_with_image(canvas.minor_gridlines);
	else
		_buffer_minor_gridlines_sync_with_image = true;
	if (major_gridlines_visible)
		gridlines_sync_with_image(canvas.major_gridlines);
	else
		_buffer_major_gridlines_sync_with_image = true;
}

void Easel::set_minor_gridlines_visibility(bool visible)
{
	if (!minor_gridlines_visible && visible)
	{
		if (_buffer_minor_gridlines_send_flat_transform)
			gridlines_send_flat_transform(canvas.minor_gridlines);
		if (_buffer_minor_gridlines_sync_with_image)
			gridlines_sync_with_image(canvas.minor_gridlines);
	}
	else if (minor_gridlines_visible && !visible)
	{
		_buffer_minor_gridlines_send_flat_transform = false;
		_buffer_minor_gridlines_sync_with_image = false;
	}
	minor_gridlines_visible = visible;
}

void Easel::set_major_gridlines_visibility(bool visible)
{
	if (!major_gridlines_visible && visible)
	{
		if (_buffer_major_gridlines_send_flat_transform)
			gridlines_send_flat_transform(canvas.major_gridlines);
		if (_buffer_major_gridlines_sync_with_image)
			gridlines_sync_with_image(canvas.major_gridlines);
	}
	else if (major_gridlines_visible && !visible)
	{
		_buffer_major_gridlines_send_flat_transform = false;
		_buffer_major_gridlines_sync_with_image = false;
	}
	major_gridlines_visible = visible;
}

void Easel::begin_panning()
{
	if (!panning_info.panning)
	{
		panning_info.initial_canvas_pos = canvas.self.transform.position;
		panning_info.initial_cursor_pos = get_app_cursor_pos();
		panning_info.panning = true;
		Machine.main_window->request_cursor(&panning_info.wh, StandardCursor::RESIZE_OMNI);
	}
}

void Easel::end_panning()
{
	if (panning_info.panning)
	{
		panning_info.panning = false;
		Machine.main_window->release_cursor(&panning_info.wh);
		Machine.main_window->release_mouse_mode(&panning_info.wh);
	}
}

void Easel::cancel_panning()
{
	if (panning_info.panning)
	{
		panning_info.panning = false;
		canvas.self.transform.position = panning_info.initial_canvas_pos;
		sync_canvas_transform();
		Machine.main_window->release_cursor(&panning_info.wh);
		Machine.main_window->release_mouse_mode(&panning_info.wh);
	}
}

void Easel::update_panning()
{
	if (panning_info.panning)
	{
		Position pan_delta = get_app_cursor_pos() - panning_info.initial_cursor_pos;
		Position pos = pan_delta + panning_info.initial_canvas_pos;
		if (Machine.main_window->is_shift_pressed())
		{
			if (std::abs(pan_delta.x) < std::abs(pan_delta.y))
				pos.x = panning_info.initial_canvas_pos.x;
			else
				pos.y = panning_info.initial_canvas_pos.y;
		}
		canvas.self.transform.position = pos;
		sync_canvas_transform();

		if (!Machine.main_window->owns_mouse_mode(&panning_info.wh) && !cursor_in_clipping())
			Machine.main_window->request_mouse_mode(&panning_info.wh, MouseMode::VIRTUAL);
		// LATER weirdly, virtual mouse is actually slower to move than visible mouse, so when virtual, scale deltas accordingly.
		// put factor in settings, and possibly even allow 2 speeds, with holding ALT or something.
	}
}

void Easel::zoom_by(float zoom)
{
	Position cursor_world;
	if (!Machine.main_window->is_ctrl_pressed())
		cursor_world = to_world_coordinates(Machine.cursor_screen_pos());

	float factor = Machine.main_window->is_shift_pressed() ? zoom_info.factor_shift : zoom_info.factor;
	float new_zoom = std::clamp(zoom_info.zoom * glm::pow(factor, zoom), zoom_info.in_min, zoom_info.in_max);
	float zoom_change = new_zoom / zoom_info.zoom;
	canvas.self.transform.scale *= zoom_change;
	Position delta_position = (canvas.self.transform.position - cursor_world) * zoom_change;
	canvas.self.transform.position = cursor_world + delta_position;

	sync_canvas_transform();
	zoom_info.zoom = new_zoom;
}

void Easel::reset_camera()
{
	zoom_info.zoom = zoom_info.initial;
	canvas.self.transform = {};
	if (canvas_image())
	{
		float fit_scale = std::min(get_app_width() / canvas_image()->buf.width, get_app_height() / canvas_image()->buf.height);
		if (fit_scale < 1.0f)
		{
			canvas.self.transform.scale *= fit_scale;
			zoom_info.zoom *= fit_scale;
		}
		else
		{
			fit_scale /= Machine.preferences.min_initial_image_window_proportion;
			if (fit_scale > 1.0f)
			{
				canvas.self.transform.scale *= fit_scale;
				zoom_info.zoom *= fit_scale;
			}
		}
	}
	sync_canvas_transform();
}
