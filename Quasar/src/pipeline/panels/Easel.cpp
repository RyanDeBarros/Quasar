#include "Easel.h"

#include <glm/gtc/type_ptr.inl>

#include "ImplUtility.h"
#include "variety/GLutility.h"
#include "user/Machine.h"
#include "BrushesPanel.h"
#include "Palette.h"
#include "../render/Uniforms.h"
#include "../render/FlatSprite.h"
#include "edit/image/PixelBufferPaths.h"

constexpr GLuint CHECKERBOARD_TSLOT = 0;
constexpr GLuint CURSOR_ERASER_TSLOT = 1;
constexpr GLuint CURSOR_SELECT_TSLOT = 2;
constexpr GLuint BRUSH_PREVIEW_TSLOT = 3;
constexpr GLuint CANVAS_SPRITE_TSLOT = 4;

Canvas::Canvas(Shader* cursor_shader)
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", std::to_string(GLC.max_texture_image_units) } }),
	Widget(_W_COUNT), brush_under_tool(&Canvas::brush_camera_tool)
{
	binfo.preview_image = std::make_shared<Image>();
	binfo.eraser_preview_image = std::make_shared<Image>();
	initialize_widget(cursor_shader);
	initialize_dot_cursor();
}

Canvas::~Canvas()
{
	delete[] dot_cursor_buf.pixels;
}

void Canvas::initialize_widget(Shader* cursor_shader)
{
	assign_widget(this, CHECKERBOARD, std::make_shared<FlatSprite>(&sprite_shader));
	fs_wget(*this, CHECKERBOARD).set_texture_slot(CHECKERBOARD_TSLOT);
	assign_widget(this, CURSOR_PENCIL, std::make_shared<W_UnitRenderable>(cursor_shader));
	ur_wget(*this, CURSOR_PENCIL).set_attribute(1, glm::value_ptr(RGBA(1.0f, 1.0f, 1.0f, 1.0f).as_vec())).send_buffer();
	assign_widget(this, CURSOR_PEN, std::make_shared<W_UnitRenderable>(cursor_shader));
	ur_wget(*this, CURSOR_PEN).set_attribute(1, glm::value_ptr(RGBA(1.0f, 1.0f, 1.0f, 1.0f).as_vec())).send_buffer();
	assign_widget(this, CURSOR_ERASER, std::make_shared<FlatSprite>(&sprite_shader));
	fs_wget(*this, CURSOR_ERASER).set_texture_slot(CURSOR_ERASER_TSLOT).image = std::make_shared<Image>(FileSystem::texture_path("eraser.png"));
	assign_widget(this, CURSOR_SELECT, std::make_shared<FlatSprite>(&sprite_shader));
	fs_wget(*this, CURSOR_SELECT).set_texture_slot(CURSOR_SELECT_TSLOT).image = std::make_shared<Image>(FileSystem::texture_path("select.png"));
	assign_widget(this, BRUSH_PREVIEW, std::make_shared<FlatSprite>(&sprite_shader));
	fs_wget(*this, BRUSH_PREVIEW).set_texture_slot(BRUSH_PREVIEW_TSLOT).image = binfo.preview_image;
	assign_widget(this, SPRITE, std::make_shared<FlatSprite>(&sprite_shader));
	fs_wget(*this, SPRITE).set_texture_slot(CANVAS_SPRITE_TSLOT);
}

void Canvas::initialize_dot_cursor()
{
	dot_cursor_buf.width = 10;
	dot_cursor_buf.height = 10;
	dot_cursor_buf.chpp = 4;
	dot_cursor_buf.pxnew();
	static const unsigned char dot_array[10*10] = {
		1,   1,   0,   0,   0,   0,   0,   0,   1,   1,
		1,   1,   0,   0,   0,   0,   0,   0,   1,   1,
		0,   0,   1,   1,   1,   1,   1,   1,   0,   0,
		0,   0,   1,   1,   1,   1,   1,   1,   0,   0,
		0,   0,   1,   1,   0,   0,   1,   1,   0,   0,
		0,   0,   1,   1,   0,   0,   1,   1,   0,   0,
		0,   0,   1,   1,   1,   1,   1,   1,   0,   0,
		0,   0,   1,   1,   1,   1,   1,   1,   0,   0,
		1,   1,   0,   0,   0,   0,   0,   0,   1,   1,
		1,   1,   0,   0,   0,   0,   0,   0,   1,   1,
	};
	for (size_t i = 0; i < dot_cursor_buf.area(); ++i)
	{
		for (size_t j = 0; j < dot_cursor_buf.chpp - 1; ++j)
			dot_cursor_buf.pixels[i * dot_cursor_buf.chpp + j] = 255 * dot_array[i];
		dot_cursor_buf.pixels[(i + 1) * dot_cursor_buf.chpp - 1] = 255;
	}
	dot_cursor = std::move(Utils::cursor_from_buffer(dot_cursor_buf));
}

void Canvas::draw()
{
	fs_wget(*this, CHECKERBOARD).draw(CHECKERBOARD_TSLOT);
	fs_wget(*this, SPRITE).draw(CANVAS_SPRITE_TSLOT);
	//if (binfo.show_preview)
		fs_wget(*this, BRUSH_PREVIEW).draw(BRUSH_PREVIEW_TSLOT);
	minor_gridlines.draw();
	major_gridlines.draw();
	if (cursor_in_canvas)
		draw_cursor();
}

void Canvas::draw_cursor()
{
	if (!pipette_ready && MBrushes->get_brush_tool() != BrushTool::CAMERA)
	{
		switch (MBrushes->get_brush_tip())
		{
		case BrushTip::PENCIL:
			ur_wget(*this, CURSOR_PENCIL).draw();
			break;
		case BrushTip::PEN:
			ur_wget(*this, CURSOR_PEN).draw();
			break;
		case BrushTip::ERASER:
			fs_wget(*this, CURSOR_ERASER).draw(CURSOR_ERASER_TSLOT);
			break;
		case BrushTip::SELECT:
			fs_wget(*this, CURSOR_SELECT).draw(CURSOR_SELECT_TSLOT);
			break;
		}
	}
}

void Canvas::send_vp(const glm::mat3& vp)
{
	Uniforms::send_matrix3(sprite_shader, "u_VP", vp);
	sync_cursor_with_widget();
}

void Canvas::sync_cursor_with_widget()
{
	sync_ur(CURSOR_PENCIL);
	sync_ur(CURSOR_PEN);
	fs_wget(*this, CURSOR_ERASER).update_transform().ur->send_buffer();
	fs_wget(*this, CURSOR_SELECT).update_transform().ur->send_buffer();
}

void Canvas::sync_ur(size_t subw)
{
	Utils::set_vertex_pos_attributes(ur_wget(*this, subw), wp_at(subw).relative_to(self.transform));
}

void Canvas::set_image(const std::shared_ptr<Image>& img)
{
	fs_wget(*this, SPRITE).image = img;
	image = img;
	sync_gfx_with_image();
}

void Canvas::set_image(std::shared_ptr<Image>&& img)
{
	fs_wget(*this, SPRITE).image = img;
	image = std::move(img);
	sync_gfx_with_image();
}

void Canvas::sync_checkerboard_with_image()
{
	FlatSprite& sprite = fs_wget(*this, SPRITE);
	if (image)
	{
		const Buffer& buf = image->buf;
		sprite.self.transform.scale = { buf.width, buf.height }; // TODO put in sync_sprite_with_image()
		sprite.update_transform();
		set_checkerboard_uv_size(0.5f * buf.width * checker_size_inv.x, 0.5f * buf.height * checker_size_inv.y);
		fs_wget(*this, CHECKERBOARD).self.transform.scale = { buf.width, buf.height };
		fs_wget(*this, CHECKERBOARD).update_transform().set_modulation(ColorFrame()).ur->send_buffer();
	}
	else
		fs_wget(*this, CHECKERBOARD).set_modulation(ColorFrame(0)).ur->send_buffer(); // LATER unnecessary because of visible data member?
}

void Canvas::sync_brush_preview_with_image()
{
	if (image)
	{
		Buffer& pbuf = binfo.preview_image->buf;
		if (!pbuf.same_dimensions_as(image->buf))
		{
			delete[] pbuf.pixels;
			pbuf = image->buf;
			pbuf.pxnew();
		}
		memset(pbuf.pixels, 0, pbuf.bytes());
		binfo.preview_image->gen_texture();
		binfo.preview_image->resend_texture();

		static const Dim eraser_arr_w = 2;
		static const Dim eraser_arr_h = 2;
		Buffer& ebuf = binfo.eraser_preview_image->buf;
		if (!ebuf.same_dimensions_as(image->buf))
		{
			delete[] ebuf.pixels;
			ebuf = image->buf;
			ebuf.width *= eraser_arr_w;
			ebuf.height *= eraser_arr_h;
			ebuf.pxnew();
		}
		// TODO use the following checkerboard multiple times (maybe even use it for eraser cursor instead?):
		static const Byte eraser_preview_arr[eraser_arr_w * eraser_arr_h * 4] = {
			255, 255, 255, 0  , 0  , 0  , 0  , 0  ,
			0  , 0  , 0  , 0  , 255, 255, 255, 0
		};
		for (Dim x = 0; x < image->buf.width; ++x)
			for (Dim y = 0; y < image->buf.height; ++y)
				for (Dim h = 0; h < eraser_arr_h; ++h)
					memcpy(ebuf.pos(eraser_arr_w * x, eraser_arr_h * y + h), eraser_preview_arr + h * eraser_arr_w * 4, eraser_arr_w * 4);
		binfo.eraser_preview_image->gen_texture();
		binfo.eraser_preview_image->resend_texture();

		fs_wget(*this, BRUSH_PREVIEW).self.transform.scale = { image->buf.width, image->buf.height };
	}
}

void Canvas::sync_gridlines_with_image()
{
	if (image)
	{
		visible = true;
		minor_gridlines.sync_with_image(image->buf.width, image->buf.height, self.transform.scale);
		major_gridlines.sync_with_image(image->buf.width, image->buf.height, self.transform.scale);
	}
}

void Canvas::sync_transform()
{
	fs_wget(*this, CHECKERBOARD).update_transform().ur->send_buffer();
	fs_wget(*this, BRUSH_PREVIEW).update_transform().ur->send_buffer();
	fs_wget(*this, SPRITE).update_transform().ur->send_buffer();
	sync_cursor_with_widget();
}

void Canvas::sync_gfx_with_image()
{
	sync_checkerboard_with_image();
	sync_brush_preview_with_image();
	sync_gridlines_with_image();
	sync_transform();
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

void Canvas::update_brush_tool()
{
	switch (MBrushes->get_brush_tool())
	{
	case BrushTool::CAMERA:
		brush_under_tool = &Canvas::brush_camera_tool;
		break;
	case BrushTool::PAINT:
		brush_under_tool = &Canvas::brush_paint_tool;
		break;
	case BrushTool::LINE:
		brush_under_tool = &Canvas::brush_line_tool;
		binfo.preview_mode = BrushActionInfo::LINE;
		break;
	case BrushTool::RECT_FILL:
		brush_under_tool = &Canvas::brush_rect_fill_tool;
		binfo.preview_mode = BrushActionInfo::RECT_FILL;
		break;
	}
}

void Canvas::update_brush_tip()
{
	switch (MBrushes->get_brush_tip())
	{
	case BrushTip::PENCIL:
		fs_wget(*this, BRUSH_PREVIEW).image = binfo.preview_image;
		break;
	case BrushTip::PEN:
		fs_wget(*this, BRUSH_PREVIEW).image = binfo.preview_image;
		break;
	case BrushTip::ERASER:
		fs_wget(*this, BRUSH_PREVIEW).image = binfo.eraser_preview_image;
		break;
	case BrushTip::SELECT:
		break;
	}
}

// TODO hover even when outside canvas for line/rect ?
void Canvas::hover_pixel_under_cursor(Position world_pos)
{
	Position local_cursor_pos = local_of(world_pos);
	Buffer& buf = image->buf;
	Position buf_cursor_pos = local_cursor_pos + 0.5f * Position(buf.width, buf.height);
	IPosition pos(buf_cursor_pos);
	if (in_diagonal_rect(pos, {}, { buf.width - 1, buf.height - 1 }))
	{
		cursor_in_canvas = true;
		if (pos != binfo.brush_pos)
		{
			if (binfo.brushing && binfo.brush_pos != IPosition{ -1, -1 })
				brush_move_to(pos);
			binfo.brush_pos = pos;
			hover_pixel_at(pixel_position(pos));
			if (cursor_state != CursorState::UP)
				brush(pos.x, pos.y);
		}
		if (MainWindow->is_alt_pressed())
		{
			MainWindow->release_cursor(&dot_cursor_wh);
			MainWindow->request_cursor(&pipette_cursor_wh, Machine.cursors.CROSSHAIR);
			pipette_ready = true;
		}
		else
		{
			MainWindow->release_cursor(&pipette_cursor_wh);
			MainWindow->request_cursor(&dot_cursor_wh, dot_cursor);
			pipette_ready = false;
		}
	}
	else
		unhover();
}

void Canvas::unhover()
{
	MainWindow->release_cursor(&dot_cursor_wh);
	MainWindow->release_cursor(&pipette_cursor_wh);
	pipette_ready = false;
	cursor_in_canvas = false;
}

void Canvas::hover_pixel_at(Position pos)
{
	wp_at(CURSOR_PENCIL).transform.position = pos;
	wp_at(CURSOR_PEN).transform.position = pos;
	wp_at(CURSOR_ERASER).transform.position = pos;
	wp_at(CURSOR_SELECT).transform.position = pos;
	sync_cursor_with_widget();
}

Position Canvas::pixel_position(IPosition pos)
{
	return Position(pos) - 0.5f * Position(image->buf.width, image->buf.height) + Position{ 0.5f, 0.5f };
}

Position Canvas::pixel_position(int x, int y)
{
	return Position(x, y) - 0.5f * Position(image->buf.width, image->buf.height) + Position{ 0.5f, 0.5f };
}

RGBA Canvas::applied_color() const
{
	return cursor_state == CursorState::DOWN_PRIMARY ? primary_color : alternate_color;
}

RGBA Canvas::color_under_cursor() const
{
	Position local_pos = local_of(MEasel->to_world_coordinates(Machine.cursor_screen_pos()));
	Position buf_cursor_pos = local_pos + 0.5f * Position(image->buf.width, image->buf.height);
	IPosition pos(buf_cursor_pos);
	Byte* pixel = image->buf.pos(pos.x, pos.y);
	PixelRGBA px{ 255, 255, 255, 255 };
	for (CHPP i = 0; i < image->buf.chpp; ++i)
		px.at(i) = pixel[i];
	return px.to_rgba();
}

PixelRGBA Canvas::pixel_color_at(IPosition pos) const
{
	return pixel_color_at(pos.x, pos.y);
}

PixelRGBA Canvas::pixel_color_at(int x, int y) const
{
	Byte* pixel = image->buf.pos(x, y);
	PixelRGBA px{ 255, 255, 255, 255 };
	for (CHPP i = 0; i < image->buf.chpp; ++i)
		px.at(i) = pixel[i];
	return px;
}

void Canvas::set_cursor_color(RGBA color)
{
	ur_wget(*this, CURSOR_PENCIL).set_attribute(1, glm::value_ptr(color.as_vec())).send_buffer();
	ur_wget(*this, CURSOR_PEN).set_attribute(1, glm::value_ptr(RGBA(color.rgb.r * color.alpha, color.rgb.g * color.alpha, color.rgb.b * color.alpha, 1.0f).as_vec())).send_buffer();
}

void Canvas::set_primary_color(RGBA color)
{
	set_cursor_color(color);
	primary_color = color;
	pric_pen_pxs.r = pric_pxs.r = color.get_pixel_r();
	pric_pen_pxs.g = pric_pxs.g = color.get_pixel_g();
	pric_pen_pxs.b = pric_pxs.b = color.get_pixel_b();
	pric_pxs.a = color.get_pixel_a();
	pric_pen_pxs.a = 255;
}

void Canvas::set_alternate_color(RGBA color)
{
	alternate_color = color;
	altc_pen_pxs.r = altc_pxs.r = color.get_pixel_r();
	altc_pen_pxs.g = altc_pxs.g = color.get_pixel_g();
	altc_pen_pxs.b = altc_pxs.b = color.get_pixel_b();
	altc_pxs.a = color.get_pixel_a();
	altc_pen_pxs.a = 255;
}

void Canvas::cursor_press(MouseButton button)
{
	if (MainWindow->is_alt_pressed())
	{
		if (cursor_in_canvas)
		{
			if (button == MouseButton::LEFT)
				MPalette->set_pri_color(color_under_cursor());
			else if (button == MouseButton::RIGHT)
				MPalette->set_alt_color(color_under_cursor());
		}
	}
	else
	{
		if (button == MouseButton::LEFT)
			cursor_state = CursorState::DOWN_PRIMARY;
		else if (button == MouseButton::RIGHT)
			cursor_state = CursorState::DOWN_ALTERNATE;
		if (cursor_in_canvas)
			brush(binfo.brush_pos.x, binfo.brush_pos.y);
	}
}

void Canvas::cursor_release()
{
	brush_submit();
	cursor_state = CursorState::UP;
	set_cursor_color(primary_color);
}

bool Canvas::cursor_cancel()
{
	if (cursor_state != CursorState::UP)
	{
		brush_cancel();
		cursor_state = CursorState::UP;
		set_cursor_color(primary_color);
		return true;
	}
	return false;
}

void Canvas::brush(int x, int y)
{
	if (!binfo.brushing)
		brush_start(x, y);
	(this->*brush_under_tool)(x, y);
}

void Canvas::brush_start(int x, int y)
{
	binfo.brushing = true;
	binfo.reset();
	binfo.starting_pos = { x, y };

	switch (MBrushes->get_brush_tool())
	{
	case BrushTool::LINE:
		brush_start_line_tool();
		break;
	case BrushTool::RECT_FILL:
		brush_start_rect_fill_tool();
		break;
	}
}

void Canvas::brush_submit()
{
	if (binfo.brushing)
	{
		switch (MBrushes->get_brush_tool())
		{
		case BrushTool::PAINT:
			if (image && !binfo.painted_colors.empty())
				Machine.history.push(std::make_shared<PaintToolAction>(image, binfo.brushing_bbox, std::move(binfo.painted_colors)));
			break;
		case BrushTool::LINE:
			brush_submit_line_tool();
			break;
		case BrushTool::RECT_FILL:
			brush_submit_rect_fill_tool();
			break;
		}
	}
	binfo.brushing = false;
	binfo.reset();
}

void Canvas::brush_cancel()
{
	binfo.brushing = false;
	binfo.reset();
}

void Canvas::brush_move_to(IPosition pos)
{
	switch (MBrushes->get_brush_tool())
	{
	case BrushTool::PAINT:
		auto& interp = binfo.interps.line;
		interp.start = binfo.brush_pos;
		interp.finish = pos;
		interp.sync_with_endpoints();
		IPosition intermediate;
		for (unsigned int i = 1; i < interp.length - 1; ++i)
		{
			interp.at(i, intermediate);
			brush(intermediate.x, intermediate.y);
		}
		break;
	}
}

void Canvas::brush_camera_tool(int x, int y)
{
	// nothing
}

// LATER here and elsewhere, add support for CHPP < 4
void Canvas::brush_paint_tool(int x, int y)
{
	Buffer image_shifted_buf = image->buf;
	image_shifted_buf.pixels += image_shifted_buf.byte_offset(x, y);
	if (x < binfo.brushing_bbox.x1)
		binfo.brushing_bbox.x1 = x;
	if (x > binfo.brushing_bbox.x2)
		binfo.brushing_bbox.x2 = x;
	if (y < binfo.brushing_bbox.y1)
		binfo.brushing_bbox.y1 = y;
	if (y > binfo.brushing_bbox.y2)
		binfo.brushing_bbox.y2 = y;
	switch (MBrushes->get_brush_tip())
	{
	case BrushTip::PENCIL:
	{
		binfo.storage_mode = BrushActionInfo::PAINTED_COLORS;
		IPosition pos{ x, y };
		PixelRGBA initial_c{ 0, 0, 0, 0 };
		PixelRGBA final_c{ 0, 0, 0, 0 };
		PixelRGBA color = cursor_state == CursorState::DOWN_PRIMARY ? pric_pxs : altc_pxs;
		float applied_alpha = applied_color().alpha;
		for (CHPP i = 0; i < image_shifted_buf.chpp; ++i)
		{
			initial_c.at(i) = image_shifted_buf.pixels[i];
			if (i < 3)
				image_shifted_buf.pixels[i] = std::clamp(roundi(color[i] * applied_alpha + image_shifted_buf.pixels[i] * (1 - applied_alpha)), 0, 255);
			else
				image_shifted_buf.pixels[i] = std::clamp(roundi(applied_alpha * 255 + image_shifted_buf.pixels[i] * (1 - applied_alpha)), 0, 255);
			final_c.at(i) = image_shifted_buf.pixels[i];
		}
		image->update_subtexture(x, y, 1, 1);
		auto iter = binfo.painted_colors.find(pos);
		if (iter == binfo.painted_colors.end())
			binfo.painted_colors[pos] = { initial_c, final_c };
		else
			iter->second.second = final_c;
		break;
	}
	case BrushTip::PEN:
	{
		binfo.storage_mode = BrushActionInfo::PAINTED_COLORS;
		IPosition pos{ x, y };
		PixelRGBA initial_c{ 0, 0, 0, 0 };
		PixelRGBA color = cursor_state == CursorState::DOWN_PRIMARY ? pric_pen_pxs : altc_pen_pxs;
		for (CHPP i = 0; i < image_shifted_buf.chpp; ++i)
		{
			initial_c.at(i) = image_shifted_buf.pixels[i];
			image_shifted_buf.pixels[i] = color[i];
		}
		image->update_subtexture(x, y, 1, 1);
		auto iter = binfo.painted_colors.find(pos);
		if (iter == binfo.painted_colors.end())
			binfo.painted_colors[pos] = { initial_c, color };
		else
			iter->second.second = color;
		break;
	}
	case BrushTip::ERASER:
	{
		binfo.storage_mode = BrushActionInfo::PAINTED_COLORS;
		Position pos{ x, y };
		PixelRGBA initial_c{ 0, 0, 0, 0 };
		PixelRGBA final_c{ 0, 0, 0, 0 };
		for (CHPP i = 0; i < image_shifted_buf.chpp; ++i)
		{
			initial_c.at(i) = image_shifted_buf.pixels[i];
			image_shifted_buf.pixels[i] = 0;
		}
		image->update_subtexture(x, y, 1, 1);
		auto iter = binfo.painted_colors.find(pos);
		if (iter == binfo.painted_colors.end())
			binfo.painted_colors[pos] = { initial_c, final_c };
		else
			iter->second.second = final_c;
		break;
	}
	case BrushTip::SELECT:
		// LATER
		break;
	}
}

// TODO seriously simplify this and brush_rect_fill_tool
void Canvas::brush_line_tool(int x, int y)
{
	switch (MBrushes->get_brush_tip())
	{
	case BrushTip::PENCIL:
	{
		binfo.storage_mode = BrushActionInfo::PAINTED_COLORS;
		binfo.painted_colors.clear();
		auto& interp = binfo.interps.line;
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos.x, pos.y);
			buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
		}
		// TODO move to interpolator
		IntBounds bbox = abs_bounds(interp.start, interp.finish);
		binfo.preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());

		interp.start = binfo.starting_pos;
		interp.finish = { x, y };
		interp.sync_with_endpoints();
		PixelRGBA new_color;
		PixelRGBA old_color;
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos);
			old_color = pixel_color_at(pos);
			new_color = applied_color().get_pixel_rgba();
			buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, new_color);
			new_color.blend_over(old_color);
			binfo.painted_colors[pos] = { new_color, old_color };
		}
		bbox = abs_bounds(interp.start, interp.finish);
		binfo.preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());
		break;
	}
	case BrushTip::PEN:
	{
		binfo.storage_mode = BrushActionInfo::PREVIEW_POSITIONS;
		binfo.preview_positions.clear();
		auto& interp = binfo.interps.line;
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos.x, pos.y);
			buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
		}
		IntBounds bbox = abs_bounds(interp.start, interp.finish);
		binfo.preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());

		interp.start = binfo.starting_pos;
		interp.finish = { x, y };
		interp.sync_with_endpoints();
		RGBA preview_color;
		PixelRGBA new_color;
		PixelRGBA old_color;
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos);
			old_color = pixel_color_at(pos);
			preview_color = applied_color();
			new_color = preview_color.get_pixel_rgba();
			buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y,
				RGBA(preview_color.rgb.r * preview_color.alpha, preview_color.rgb.g * preview_color.alpha, preview_color.rgb.b * preview_color.alpha, 1.0f).get_pixel_rgba());
			binfo.preview_positions[pos] = old_color;
		}
		bbox = abs_bounds(interp.start, interp.finish);
		binfo.preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());
		break;
	}
	case BrushTip::ERASER:
	{
		binfo.storage_mode = BrushActionInfo::PREVIEW_POSITIONS;
		binfo.preview_positions.clear();
		auto& interp = binfo.interps.line;
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos);
			// TODO here and for rect fill, use eraser_arr_w/h instead of 2
			buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, 2, 2);
		}
		IntBounds bbox = abs_bounds(interp.start, interp.finish, 2, 2);
		binfo.eraser_preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());

		interp.start = binfo.starting_pos;
		interp.finish = { x, y };
		interp.sync_with_endpoints();
		RGBA preview_color;
		PixelRGBA new_color;
		PixelRGBA old_color;
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos);
			old_color = pixel_color_at(pos);
			preview_color = applied_color();
			new_color = preview_color.get_pixel_rgba();
			buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, 2, 2);
			binfo.preview_positions[pos] = old_color;
		}
		bbox = abs_bounds(interp.start, interp.finish, 2, 2);
		binfo.eraser_preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());
		break;
	}
	case BrushTip::SELECT:
		// LATER
		break;
	}
}

void Canvas::brush_start_line_tool()
{
	binfo.show_preview = true;
}

void Canvas::brush_submit_line_tool()
{
	// TODO
	//if (binfo.brush_pos != IPosition{ -1, -1 })
		//brush(binfo.brush_pos.x, binfo.brush_pos.y);
	if (image)
	{
		switch (MBrushes->get_brush_tip())
		{
		case BrushTip::PENCIL:
			if (!binfo.painted_colors.empty())
				Machine.history.execute(std::make_shared<OneColorPencilAction>(image, binfo.starting_pos, binfo.brush_pos, std::move(binfo.painted_colors)));
			break;
		case BrushTip::PEN:
			if (!binfo.preview_positions.empty())
				Machine.history.execute(std::make_shared<OneColorPenAction>(image, applied_color().get_pixel_rgba(), binfo.starting_pos, binfo.brush_pos, std::move(binfo.preview_positions)));
			break;
		case BrushTip::ERASER:
			if (!binfo.preview_positions.empty())
				Machine.history.execute(std::make_shared<OneColorPenAction>(image, PixelRGBA{ 0, 0, 0, 0 }, binfo.starting_pos, binfo.brush_pos, std::move(binfo.preview_positions)));
			break;
		case BrushTip::SELECT:
			// LATER
			break;
		}
	}
}

void Canvas::brush_rect_fill_tool(int x, int y)
{
	if (MBrushes->get_brush_tip() != BrushTip::SELECT)
	{
		DiscreteRectFillInterpolator new_interp{ binfo.starting_pos, { x, y } };
		new_interp.sync_with_endpoints();
		auto& diff = binfo.interp_diffs.rect_fill;
		diff.first = binfo.interps.rect_fill;
		diff.second = new_interp;
		diff.sync_with_interpolators();
		IPosition pos{};
		// TODO move functors to data members. also rename painted_colors to 2-color storage, and preview_positions to 1-color storage.
		auto painted_colors_storage_remove = [&pc = binfo.painted_colors](IPosition pos) { pc.erase(pos); };
		auto preview_positions_storage_remove = [&pp = binfo.preview_positions](IPosition pos) { pp.erase(pos); };
		std::function<void(IPosition)> storage_remove;
		if (binfo.storage_mode == BrushActionInfo::PAINTED_COLORS)
			storage_remove = painted_colors_storage_remove;
		else if (binfo.storage_mode == BrushActionInfo::PREVIEW_POSITIONS)
			storage_remove = preview_positions_storage_remove;

		for (unsigned int i = 0; i < diff.remove_length; ++i)
		{
			diff.remove_at(i, pos.x, pos.y);
			if (MBrushes->get_brush_tip() & BrushTip::ERASER)
				buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, 2, 2);
			else
				buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
			storage_remove(pos);
		}

		auto painted_colors_storage_insert = [&pc = binfo.painted_colors](IPosition pos, PixelRGBA new_color, PixelRGBA old_color) {
			new_color.blend_over(old_color);
			pc[pos] = { new_color, old_color };
			};
		auto preview_positions_storage_insert = [&pp = binfo.preview_positions](IPosition pos, PixelRGBA new_color, PixelRGBA old_color) {
			pp[pos] = old_color;
			};
		std::function<void(IPosition, PixelRGBA, PixelRGBA)> storage_insert;
		if (binfo.storage_mode == BrushActionInfo::PAINTED_COLORS)
			storage_insert = painted_colors_storage_insert;
		else if (binfo.storage_mode == BrushActionInfo::PREVIEW_POSITIONS)
			storage_insert = preview_positions_storage_insert;

		RGBA preview_color;
		PixelRGBA old_color;
		PixelRGBA new_color;
		for (unsigned int i = 0; i < diff.insert_length; ++i)
		{
			diff.insert_at(i, pos.x, pos.y);
			old_color = pixel_color_at(pos.x, pos.y);
			preview_color = applied_color();
			new_color = preview_color.get_pixel_rgba();
			if (MBrushes->get_brush_tip() & BrushTip::ERASER)
				buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, 2, 2);
			else
				buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y,
					RGBA(preview_color.rgb.r * preview_color.alpha, preview_color.rgb.g * preview_color.alpha, preview_color.rgb.b * preview_color.alpha, 1.0f).get_pixel_rgba());

			storage_insert(pos, new_color, old_color);
		}
		if (MBrushes->get_brush_tip() & BrushTip::ERASER)
		{
			binfo.eraser_preview_image->update_subtexture(diff.first_bbox.x1 * 2, diff.first_bbox.y1 * 2, diff.first_bbox.width_no_abs() * 2, diff.first_bbox.height_no_abs() * 2);
			binfo.eraser_preview_image->update_subtexture(diff.second_bbox.x1 * 2, diff.second_bbox.y1 * 2, diff.second_bbox.width_no_abs() * 2, diff.second_bbox.height_no_abs() * 2);
		}
		else
		{
			binfo.preview_image->update_subtexture(diff.first_bbox.x1, diff.first_bbox.y1, diff.first_bbox.width_no_abs(), diff.first_bbox.height_no_abs());
			binfo.preview_image->update_subtexture(diff.second_bbox.x1, diff.second_bbox.y1, diff.second_bbox.width_no_abs(), diff.second_bbox.height_no_abs());
		}
		binfo.interps.rect_fill = new_interp;
		binfo.interps.rect_fill.sync_with_endpoints();
	}
	else
	{
		// LATER
	}
}

void Canvas::brush_start_rect_fill_tool()
{
	binfo.show_preview = true;
	binfo.interps.rect_fill.start = binfo.interps.rect_fill.finish = binfo.starting_pos;
	binfo.interps.rect_fill.sync_with_endpoints();
	if (binfo.starting_pos != IPosition{ -1, -1 }) // TODO if starting brush, would this really ever fail?
	{
		PixelRGBA old_color = pixel_color_at(binfo.starting_pos.x, binfo.starting_pos.y);
		PixelRGBA new_color = applied_color().get_pixel_rgba();
		if (MBrushes->get_brush_tip() & (BrushTip::PENCIL | BrushTip::PEN))
		{
			buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, new_color);
			binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
		}
		else if (MBrushes->get_brush_tip() & BrushTip::ERASER)
		{
			buffer_set_rect_alpha(binfo.eraser_preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, 1, 1, 255, 2, 2);
			binfo.eraser_preview_image->update_subtexture(binfo.starting_pos.x * 2, binfo.starting_pos.y * 2, 2, 2);
		}
		if (MBrushes->get_brush_tip() & BrushTip::PENCIL)
		{
			new_color.blend_over(old_color);
			binfo.storage_mode = BrushActionInfo::PAINTED_COLORS;
		}
		else if (MBrushes->get_brush_tip() & (BrushTip::PEN | BrushTip::ERASER))
			binfo.storage_mode = BrushActionInfo::PREVIEW_POSITIONS;
		if (binfo.storage_mode == BrushActionInfo::PAINTED_COLORS)
			binfo.painted_colors[binfo.starting_pos] = { new_color, old_color };
		else if (binfo.storage_mode == BrushActionInfo::PREVIEW_POSITIONS)
			binfo.preview_positions[binfo.starting_pos] = new_color;
	}
}

void Canvas::brush_submit_rect_fill_tool()
{
	// TODO this comment lags:
	//if (binfo.brush_pos != IPosition{ -1, -1 })
		//hover_pixel_under_cursor(Machine.easel_cursor_world_pos());
	if (image)
	{
		switch (MBrushes->get_brush_tip())
		{
		case BrushTip::PENCIL:
			if (!binfo.painted_colors.empty())
				Machine.history.execute(std::make_shared<OneColorPencilAction>(image, binfo.starting_pos, binfo.brush_pos, std::move(binfo.painted_colors)));
			break;
		case BrushTip::PEN:
			if (!binfo.preview_positions.empty())
				Machine.history.execute(std::make_shared<OneColorPenAction>(image, applied_color().get_pixel_rgba(), binfo.starting_pos, binfo.brush_pos, std::move(binfo.preview_positions)));
			break;
		case BrushTip::ERASER:
			if (!binfo.preview_positions.empty())
				Machine.history.execute(std::make_shared<OneColorPenAction>(image, PixelRGBA{ 0, 0, 0, 0 }, binfo.starting_pos, binfo.brush_pos, std::move(binfo.preview_positions)));
			break;
		case BrushTip::SELECT:
			// LATER
			break;
		}
	}
}

void Canvas::BrushActionInfo::reset()
{
	brushing_bbox.x1 = INT_MAX;
	brushing_bbox.x2 = INT_MIN;
	brushing_bbox.y1 = INT_MAX;
	brushing_bbox.y2 = INT_MIN;
	brush_pos = { -1, -1 };
	starting_pos = { -1, -1 };
	if (show_preview)
	{
		switch (preview_mode)
		{
		case LINE:
		{
			// TODO brush tool/tip should be data member of BrushActionInfo, possibly renamed BrushInfo
			if (MBrushes->get_brush_tip() & (BrushTip::PENCIL | BrushTip::PEN))
			{
				IPosition pos;
				for (unsigned int i = 0; i < interps.line.length; ++i)
				{
					interps.line.at(i, pos);
					buffer_set_pixel_alpha(preview_image->buf, pos.x, pos.y, 0);
				}
				IntBounds bbox = abs_bounds(interps.line.start, interps.line.finish);
				preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());
			}
			else if (MBrushes->get_brush_tip() & BrushTip::ERASER)
			{
				IPosition pos;
				for (unsigned int i = 0; i < interps.line.length; ++i)
				{
					interps.line.at(i, pos);
					buffer_set_rect_alpha(eraser_preview_image->buf, 2 * pos.x, 2 * pos.y, 2, 2, 0);
				}
				IntBounds bbox = abs_bounds(interps.line.start, interps.line.finish);
				eraser_preview_image->update_subtexture(2 * bbox.x1, 2 * bbox.y1, 2 * bbox.width_no_abs(), 2 * bbox.height_no_abs());
			}
			break;
		}
		case RECT_FILL:
		{
			IntBounds bbox = interp_diffs.rect_fill.second_bbox;
			UprightRect upr;

			upr.x0 = bbox.x1;
			upr.x1 = bbox.x2;
			upr.y0 = bbox.y1;
			upr.y1 = bbox.y2;

			if (MBrushes->get_brush_tip() & (BrushTip::PENCIL | BrushTip::PEN))
			{
				iterate_path(upr, [&buf = preview_image->buf](PathIterator& pit) { buffer_set_pixel_alpha(buf, pit.x, pit.y, 0); });
				preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());
			}
			else if (MBrushes->get_brush_tip() & BrushTip::ERASER)
			{
				iterate_path(upr, [&buf = eraser_preview_image->buf](PathIterator& pit) { buffer_set_rect_alpha(buf, pit.x, pit.y, 1, 1, 0, 2, 2); });
				eraser_preview_image->update_subtexture(bbox.x1 * 2, bbox.y1 * 2, bbox.width_no_abs() * 2, bbox.height_no_abs() * 2);
			}
			break;
		}
		}
	}
	show_preview = false;
	painted_colors.clear();
	preview_positions.clear();
}

Easel::Easel()
	: color_square_shader(FileSystem::shader_path("color_square.vert"), FileSystem::shader_path("color_square.frag")), widget(_W_COUNT)
{
	connect_input_handlers();
}

void Easel::initialize()
{
	initialize_widget();
}

void Easel::initialize_widget()
{
	assign_widget(&widget, BACKGROUND, std::make_shared<W_UnitRenderable>(&color_square_shader));
	ur_wget(widget, BACKGROUND).set_attribute(1, glm::value_ptr(RGBA(HSV(0.5f, 0.15f, 0.15f).to_rgb(), 0.5f).as_vec())).send_buffer();

	assign_widget(&widget, CANVAS, std::make_shared<Canvas>(&color_square_shader));
	Canvas& cnvs = canvas();
	cnvs.create_checkerboard_image();
	cnvs.set_image(nullptr);
	cnvs.checker1 = Machine.preferences.checker1;
	cnvs.checker2 = Machine.preferences.checker2;
	cnvs.set_checker_size(Machine.preferences.checker_size);
	cnvs.sync_checkerboard_colors();
}

void Easel::connect_input_handlers()
{
	mb_handler.callback = [this](const MouseButtonEvent& mb) {
		if (ImGui::GetIO().WantCaptureMouse)
			return;
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == IAction::PRESS && cursor_in_clipping())
			{
				mb.consumed = true;
				begin_panning();
			}
			else if (mb.action == IAction::RELEASE)
				end_panning();
		}
		else if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == IAction::PRESS && cursor_in_clipping())
			{
				mb.consumed = true;
				if (MainWindow->is_key_pressed(Key::SPACE))
					begin_panning();
				else
					canvas().cursor_press(mb.button);
			}
			else if (mb.action == IAction::RELEASE)
			{
				end_panning();
				canvas().cursor_release();
			}
		}
		else if (mb.button == MouseButton::RIGHT)
		{
			if (mb.action == IAction::PRESS)
			{
				mb.consumed = true;
				canvas().cursor_press(mb.button);
			}
			else if (mb.action == IAction::RELEASE)
				canvas().cursor_release();
		}
		};
	// LATER add handler for when mouse is already pressed, and then space is pressed to pan.
	key_handler.callback = [this](const KeyEvent& k) {
		if (k.action == IAction::PRESS && k.key == Key::ESCAPE && canvas().cursor_cancel())
			k.consumed = true;
		};
	scroll_handler.callback = [this](const ScrollEvent& s) {
		if (!panning_info.panning && cursor_in_clipping())
		{
			s.consumed = true;
			zoom_by(s.yoff);
		}
		};
}

void Easel::draw()
{
	ur_wget(widget, BACKGROUND).draw();
	Canvas& cnvs = canvas();
	if (cnvs.visible)
		cnvs.draw();
}

void Easel::_send_view()
{
	vp = vp_matrix();
	canvas().send_vp(vp);
	Uniforms::send_matrix3(color_square_shader, "u_VP", vp);
	Uniforms::send_matrix3(canvas().minor_gridlines.shader, "u_VP", vp);
	Uniforms::send_matrix3(canvas().major_gridlines.shader, "u_VP", vp);
	unbind_shader();
	sync_widget();
}

void Easel::process()
{
	update_panning();
	if (cursor_in_clipping())
		canvas().hover_pixel_under_cursor(Machine.cursor_world_pos(glm::inverse(vp)));
	else
		canvas().unhover();
}

void Easel::sync_widget()
{
	widget.wp_at(BACKGROUND).transform.scale = get_app_size();
	sync_ur(BACKGROUND);
	canvas().sync_cursor_with_widget();
}

void Easel::sync_ur(size_t subw)
{
	Utils::set_vertex_pos_attributes(ur_wget(widget, subw), widget.wp_at(subw).relative_to(widget.self.transform));
}

void Easel::sync_canvas_transform()
{
	canvas().sync_transform();
	canvas().minor_gridlines.send_flat_transform(canvas().self.transform);
	canvas().major_gridlines.send_flat_transform(canvas().self.transform);
}

const Image* Easel::canvas_image() const
{
	return canvas().image.get();
}

Image* Easel::canvas_image()
{
	return canvas().image.get();
}

bool Easel::minor_gridlines_are_visible() const
{
	return canvas().minor_gridlines.visible();
}

void Easel::set_minor_gridlines_visibility(bool visible)
{
	canvas().minor_gridlines.set_visible(visible, canvas().self.transform, canvas().image->buf.width, canvas().image->buf.height);
}

bool Easel::major_gridlines_are_visible() const
{
	return canvas().major_gridlines.visible();
}

void Easel::set_major_gridlines_visibility(bool visible)
{
	canvas().major_gridlines.set_visible(visible, canvas().self.transform, canvas().image->buf.width, canvas().image->buf.height);
}

void Easel::begin_panning()
{
	if (!panning_info.panning)
	{
		panning_info.initial_canvas_pos = canvas().self.transform.position;
		panning_info.initial_cursor_pos = get_app_cursor_pos();
		panning_info.panning = true;
		MainWindow->request_cursor(&panning_info.wh, Machine.cursors.RESIZE_OMNI);
	}
}

void Easel::end_panning()
{
	if (panning_info.panning)
	{
		panning_info.panning = false;
		MainWindow->release_cursor(&panning_info.wh);
		MainWindow->release_mouse_mode(&panning_info.wh);
	}
}

void Easel::cancel_panning()
{
	if (panning_info.panning)
	{
		panning_info.panning = false;
		canvas().self.transform.position = panning_info.initial_canvas_pos;
		sync_canvas_transform();
		MainWindow->release_cursor(&panning_info.wh);
		MainWindow->release_mouse_mode(&panning_info.wh);
	}
}

void Easel::update_panning()
{
	if (panning_info.panning)
	{
		Position pan_delta = get_app_cursor_pos() - panning_info.initial_cursor_pos;
		Position pos = pan_delta + panning_info.initial_canvas_pos;
		if (MainWindow->is_shift_pressed())
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

		if (!MainWindow->owns_mouse_mode(&panning_info.wh) && !cursor_in_clipping())
			MainWindow->request_mouse_mode(&panning_info.wh, MouseMode::VIRTUAL);
		// LATER weirdly, virtual mouse is actually slower to move than visible mouse, so when virtual, scale deltas accordingly.
		// put factor in settings, and possibly even allow 2 speeds, with holding ALT or something.
	}
}

void Easel::zoom_by(float zoom)
{
	Position cursor_world;
	if (!MainWindow->is_ctrl_pressed())
		cursor_world = to_world_coordinates(Machine.cursor_screen_pos());

	float factor = MainWindow->is_shift_pressed() ? zoom_info.factor_shift : zoom_info.factor;
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

void Easel::flip_image_horizontally()
{
	struct FlipHorizontallyAction : public ActionBase
	{
		Easel* easel;
		FlipHorizontallyAction(Easel* easel) : easel(easel) { weight = sizeof(FlipHorizontallyAction); }
		virtual void forward() override { if (easel) easel->canvas_image()->flip_horizontally(); }
		virtual void backward() override { if (easel) easel->canvas_image()->flip_horizontally(); }
		QUASAR_ACTION_EQUALS_OVERRIDE(FlipHorizontallyAction)
	};
	struct FlipHorizontallyAction_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		FlipHorizontallyAction_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(FlipHorizontallyAction_Perf);
			buf = easel->canvas_image()->buf;
			buf.pxnew();
			subbuffer_copy(buf, easel->canvas_image()->buf);
			buf.flip_horizontally();
			weight += buf.bytes();
		}
		~FlipHorizontallyAction_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->update_texture();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(FlipHorizontallyAction_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<FlipHorizontallyAction_Perf>(this));
	else
		Machine.history.execute(std::make_shared<FlipHorizontallyAction>(this));
}

void Easel::flip_image_vertically()
{
	struct FlipVerticallyAction : public ActionBase
	{
		Easel* easel;
		FlipVerticallyAction(Easel* easel) : easel(easel) { weight = sizeof(FlipVerticallyAction); }
		virtual void forward() override { if (easel) easel->canvas_image()->flip_vertically(); }
		virtual void backward() override { if (easel) easel->canvas_image()->flip_vertically(); }
		QUASAR_ACTION_EQUALS_OVERRIDE(FlipVerticallyAction)
	};
	struct FlipVerticallyAction_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		FlipVerticallyAction_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(FlipVerticallyAction_Perf);
			buf = easel->canvas_image()->buf;
			buf.pxnew();
			subbuffer_copy(buf, easel->canvas_image()->buf);
			buf.flip_vertically();
			weight += buf.bytes();
		}
		~FlipVerticallyAction_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->update_texture();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(FlipVerticallyAction_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<FlipVerticallyAction_Perf>(this));
	else
		Machine.history.execute(std::make_shared<FlipVerticallyAction>(this));
}

void Easel::rotate_image_90()
{
	struct Rotate90Action : public ActionBase
	{
		Easel* easel;
		Rotate90Action(Easel* easel) : easel(easel) { weight = sizeof(Rotate90Action); }
		virtual void forward() override { if (easel) { easel->canvas_image()->rotate_90_del_old(); easel->canvas().sync_gfx_with_image(); } }
		virtual void backward() override { if (easel) { easel->canvas_image()->rotate_270_del_old(); easel->canvas().sync_gfx_with_image(); } }
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate90Action)
	};
	struct Rotate90Action_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		Rotate90Action_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(Rotate90Action_Perf);
			buf = easel->canvas_image()->buf.rotate_90_ret_new();
			weight += buf.bytes();
		}
		~Rotate90Action_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->resend_texture();
				easel->canvas().sync_gfx_with_image();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate90Action_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<Rotate90Action_Perf>(this));
	else
		Machine.history.execute(std::make_shared<Rotate90Action>(this));
}

void Easel::rotate_image_180()
{
	struct Rotate180Action : public ActionBase
	{
		Easel* easel;
		Rotate180Action(Easel* easel) : easel(easel) { weight = sizeof(Rotate180Action); }
		virtual void forward() override { if (easel) easel->canvas_image()->rotate_180(); }
		virtual void backward() override { if (easel) easel->canvas_image()->rotate_180(); }
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate180Action)
	};
	struct Rotate180Action_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		Rotate180Action_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(Rotate180Action_Perf);
			buf = easel->canvas_image()->buf;
			buf.pxnew();
			subbuffer_copy(buf, easel->canvas_image()->buf);
			buf.rotate_180();
			weight += buf.bytes();
		}
		~Rotate180Action_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->update_texture();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate180Action_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<Rotate180Action_Perf>(this));
	else
		Machine.history.execute(std::make_shared<Rotate180Action>(this));
}

void Easel::rotate_image_270()
{
	struct Rotate270Action : public ActionBase
	{
		Easel* easel;
		Rotate270Action(Easel* easel) : easel(easel) { weight = sizeof(Rotate270Action); }
		virtual void forward() override { if (easel) { easel->canvas_image()->rotate_270_del_old(); easel->canvas().sync_gfx_with_image(); } }
		virtual void backward() override { if (easel) { easel->canvas_image()->rotate_90_del_old(); easel->canvas().sync_gfx_with_image(); } }
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate270Action)
	};
	struct Rotate270Action_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		Rotate270Action_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(Rotate270Action_Perf);
			buf = easel->canvas_image()->buf.rotate_270_ret_new();
			weight += buf.bytes();
		}
		~Rotate270Action_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->resend_texture();
				easel->canvas().sync_gfx_with_image();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate270Action_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<Rotate270Action_Perf>(this));
	else
		Machine.history.execute(std::make_shared<Rotate270Action>(this));
}
