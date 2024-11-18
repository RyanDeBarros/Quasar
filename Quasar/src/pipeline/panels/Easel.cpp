#include "Easel.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"
#include "user/Machine.h"
#include "../render/Uniforms.h"
#include "../render/FlatSprite.h"

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
		_nonobstructing = false;
		return;
	}
	_nonobstructing = true;

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
	if (_visible && _nonobstructing)
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

void Gridlines::send_buffer() const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, num_vertices() * shader.stride * sizeof(GLfloat), varr, GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

void Gridlines::send_flat_transform(FlatTransform canvas_transform) const
{
	if (_visible)
	{
		Uniforms::send_4(shader, "u_FlatTransform", canvas_transform.packed());
		update_scale(canvas_transform.scale);
		send_buffer();
		_send_flat_transform = false;
	}
	else
		_send_flat_transform = true;
}

void Gridlines::sync_with_image(const Buffer& buf, Scale canvas_scale)
{
	if (_visible)
	{
		width = buf.width;
		height = buf.height;
		resize_grid(canvas_scale);
		send_buffer();
		_sync_with_image = false;
	}
	else
		_sync_with_image = true;
}

void Gridlines::set_visible(bool visible, const Canvas& canvas)
{
	if (!_visible && visible)
	{
		_visible = true;
		if (_send_flat_transform)
			send_flat_transform(canvas.self.transform);
		if (_sync_with_image)
			sync_with_image(fs_wget(canvas, Canvas::SPRITE).image->buf, canvas.self.transform.scale);
	}
	else if (_visible && !visible)
	{
		_visible = false;
		_send_flat_transform = false;
		_sync_with_image = false;
	}
}

Canvas::Canvas(Shader* sprite_shader)
	: Widget(_W_COUNT)
{
	assign_widget(this, CHECKERBOARD, std::make_shared<FlatSprite>(sprite_shader));
	assign_widget(this, SPRITE, std::make_shared<FlatSprite>(sprite_shader));
}

void Canvas::create_checkerboard_image()
{
	auto img = std::make_shared<Image>();
	img->buf.width = 2;
	img->buf.height = 2;
	img->buf.chpp = 4;
	img->buf.pxnew();
	img->gen_texture();
	wp_at(CHECKERBOARD).transform.scale = { img->buf.width, img->buf.height };
	fs_wget(*this, CHECKERBOARD).image = std::move(img);
	fs_wget(*this, CHECKERBOARD).update_transform();
	set_checkerboard_uv_size(0, 0);
}

void Canvas::sync_checkerboard_colors() const
{
	const FlatSprite& checkerboard = fs_wget(*this, CHECKERBOARD);
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
	const FlatSprite& checkerboard = fs_wget(*this, CHECKERBOARD);
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
	fs_wget(*this, CHECKERBOARD).set_uvs(Bounds{ 0.0f, width, 0.0f, height });
}

void Canvas::set_checker_size(glm::ivec2 checker_size)
{
	checker_size_inv = { 1.0f / checker_size.x, 1.0f / checker_size.y };
	major_gridlines.line_spacing = checker_size;
}

void Canvas::set_image(const std::shared_ptr<Image>& img)
{
	FlatSprite& sprite = fs_wget(*this, SPRITE);
	sprite.image = img;
	if (sprite.image)
	{
		const Buffer& buf = sprite.image->buf;
		sprite.self.transform.scale = { buf.width, buf.height };
		sprite.update_transform();
		set_checkerboard_uv_size(0.5f * buf.width * checker_size_inv.x, 0.5f * buf.height * checker_size_inv.y);
		fs_wget(*this, CHECKERBOARD).self.transform.scale = { buf.width, buf.height };
		fs_wget(*this, CHECKERBOARD).update_transform().set_modulation(ColorFrame()).ur->send_buffer();
	}
	else
		fs_wget(*this, CHECKERBOARD).set_modulation(ColorFrame(0)).ur->send_buffer();
}

void Canvas::set_image(std::shared_ptr<Image>&& img)
{
	FlatSprite& sprite = fs_wget(*this, SPRITE);
	sprite.image = std::move(img);
	if (sprite.image)
	{
		const Buffer& buf = sprite.image->buf;
		sprite.self.transform.scale = { buf.width, buf.height };
		sprite.update_transform();
		set_checkerboard_uv_size(0.5f * buf.width * checker_size_inv.x, 0.5f * buf.height * checker_size_inv.y);
		fs_wget(*this, CHECKERBOARD).self.transform.scale = { buf.width, buf.height };
		fs_wget(*this, CHECKERBOARD).update_transform().set_modulation(ColorFrame()).ur->send_buffer();
	}
	else
		fs_wget(*this, CHECKERBOARD).set_modulation(ColorFrame(0)).ur->send_buffer();
}

void Canvas::sync_transform()
{
	fs_wget(*this, SPRITE).update_transform().ur->send_buffer();
	fs_wget(*this, CHECKERBOARD).update_transform().ur->send_buffer();
}

constexpr GLuint CHECKERBOARD_TSLOT = 0;
constexpr GLuint CANVAS_SPRITE_TSLOT = 1;

Easel::Easel()
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", std::to_string(GLC.max_texture_image_units) }}),
	bkg_shader(FileSystem::shader_path("color_square.vert"), FileSystem::shader_path("color_square.frag")), widget(_W_COUNT)
{
	initialize_widget();
}

void Easel::initialize_widget()
{
	assign_widget(&widget, BACKGROUND, std::make_shared<W_UnitRenderable>(&bkg_shader));
	ur_wget(widget, BACKGROUND).set_attribute(1, glm::value_ptr(RGBA(HSV(0.5f, 0.15f, 0.15f).to_rgb(), 0.5f).as_vec()));
	ur_wget(widget, BACKGROUND).send_buffer();

	assign_widget(&widget, CANVAS, std::make_shared<Canvas>(&sprite_shader));
	Canvas& cnvs = canvas();
	cnvs.create_checkerboard_image();
	cnvs.set_image(nullptr);
	cnvs.checker1 = Machine.preferences.checker1;
	cnvs.checker2 = Machine.preferences.checker2;
	cnvs.set_checker_size(Machine.preferences.checker_size);
	cnvs.sync_checkerboard_colors();
	// TODO put these in Canvas ???
	fs_wget(cnvs, Canvas::CHECKERBOARD).set_texture_slot(CHECKERBOARD_TSLOT);
	fs_wget(cnvs, Canvas::SPRITE).set_texture_slot(CANVAS_SPRITE_TSLOT);
}

void Easel::draw()
{
	ur_wget(widget, BACKGROUND).draw();
	Canvas& cnvs = canvas();
	if (cnvs.visible)
	{
		fs_wget(cnvs, Canvas::CHECKERBOARD).draw(CHECKERBOARD_TSLOT);
		fs_wget(cnvs, Canvas::SPRITE).draw(CANVAS_SPRITE_TSLOT);
		cnvs.minor_gridlines.draw();
		cnvs.major_gridlines.draw();
	}
}

void Easel::_send_view()
{
	glm::mat3 cameraVP = vp_matrix();
	Uniforms::send_matrix3(sprite_shader, "u_VP", cameraVP);
	Uniforms::send_matrix3(bkg_shader, "u_VP", cameraVP);
	Uniforms::send_matrix3(canvas().minor_gridlines.shader, "u_VP", cameraVP);
	Uniforms::send_matrix3(canvas().major_gridlines.shader, "u_VP", cameraVP);
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
	canvas().sync_transform();
	canvas().minor_gridlines.send_flat_transform(canvas().self.transform);
	canvas().major_gridlines.send_flat_transform(canvas().self.transform);
}

void Easel::set_canvas_image(const std::shared_ptr<Image>& img)
{
	bool nonnull = img.get();
	canvas().set_image(img);
	if (nonnull)
	{
		canvas().visible = true;
		canvas().minor_gridlines.sync_with_image(fs_wget(canvas(), Canvas::SPRITE).image->buf, canvas().self.transform.scale);
		canvas().major_gridlines.sync_with_image(fs_wget(canvas(), Canvas::SPRITE).image->buf, canvas().self.transform.scale);
	}
}

void Easel::set_canvas_image(std::shared_ptr<Image>&& img)
{
	bool nonnull = img.get();
	canvas().set_image(std::move(img));
	if (nonnull)
	{
		canvas().visible = true;
		canvas().minor_gridlines.sync_with_image(fs_wget(canvas(), Canvas::SPRITE).image->buf, canvas().self.transform.scale);
		canvas().major_gridlines.sync_with_image(fs_wget(canvas(), Canvas::SPRITE).image->buf, canvas().self.transform.scale);
	}
}

void Easel::update_canvas_image()
{
	set_canvas_image(fs_wget(canvas(), Canvas::SPRITE).image);
	canvas().sync_transform();
}

Image* Easel::canvas_image() const
{
	return fs_wget(canvas(), Canvas::SPRITE).image.get();
}

bool Easel::minor_gridlines_are_visible() const
{
	return canvas().minor_gridlines.visible();
}

void Easel::set_minor_gridlines_visibility(bool visible)
{
	canvas().minor_gridlines.set_visible(visible, canvas());
}

bool Easel::major_gridlines_are_visible() const
{
	return canvas().major_gridlines.visible();
}

void Easel::set_major_gridlines_visibility(bool visible)
{
	canvas().major_gridlines.set_visible(visible, canvas());
}

void Easel::begin_panning()
{
	if (!panning_info.panning)
	{
		panning_info.initial_canvas_pos = canvas().self.transform.position;
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
		canvas().self.transform.position = panning_info.initial_canvas_pos;
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
		if (canvas().self.transform.position != pos)
		{
			canvas().self.transform.position = pos;
			sync_canvas_transform();
		}

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
	canvas().self.transform.scale *= zoom_change;
	Position delta_position = (canvas().self.transform.position - cursor_world) * zoom_change;
	canvas().self.transform.position = cursor_world + delta_position;

	sync_canvas_transform();
	zoom_info.zoom = new_zoom;
}

void Easel::reset_camera()
{
	zoom_info.zoom = zoom_info.initial;
	canvas().self.transform = {};
	if (canvas_image())
	{
		float fit_scale = std::min(get_app_width() / canvas_image()->buf.width, get_app_height() / canvas_image()->buf.height);
		if (fit_scale < 1.0f)
		{
			canvas().self.transform.scale *= fit_scale;
			zoom_info.zoom *= fit_scale;
		}
		else
		{
			fit_scale /= Machine.preferences.min_initial_image_window_proportion;
			if (fit_scale > 1.0f)
			{
				canvas().self.transform.scale *= fit_scale;
				zoom_info.zoom *= fit_scale;
			}
		}
	}
	sync_canvas_transform();
}
