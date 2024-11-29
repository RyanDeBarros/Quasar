#include "Easel.h"

#include <glm/gtc/type_ptr.inl>

#include "CanvasBrushImpl.h"
#include "ImplUtility.h"
#include "variety/GLutility.h"
#include "user/Machine.h"
#include "BrushesPanel.h"
#include "Palette.h"
#include "../render/Uniforms.h"
#include "../render/FlatSprite.h"

constexpr GLuint CHECKERBOARD_TSLOT = 0;
constexpr GLuint CURSOR_ERASER_TSLOT = 1;
constexpr GLuint CURSOR_SELECT_TSLOT = 2;
constexpr GLuint BRUSH_PREVIEW_TSLOT = 3;
constexpr GLuint CANVAS_SPRITE_TSLOT = 4;

Canvas::Canvas(Shader* cursor_shader)
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", std::to_string(GLC.max_texture_image_units) } }),
	Widget(_W_COUNT), brush_under_tool_and_tip(&CBImpl::Camera::brush), eraser_cursor_img(std::make_shared<Image>())
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
	fs_wget(*this, CURSOR_ERASER).set_texture_slot(CURSOR_ERASER_TSLOT).image = eraser_cursor_img;
	initialize_eraser_cursor_img();
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
	static const Byte dot_array[10*10] = {
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

void Canvas::initialize_eraser_cursor_img()
{
	eraser_cursor_img->buf.width = 2;
	eraser_cursor_img->buf.height = 2;
	eraser_cursor_img->buf.chpp = 3;
	eraser_cursor_img->buf.pxnew();
	for (CHPP i = 0; i < eraser_cursor_img->buf.chpp; ++i)
	{
		eraser_cursor_img->buf.pixels[0 * eraser_cursor_img->buf.chpp + i] = 255;
		eraser_cursor_img->buf.pixels[1 * eraser_cursor_img->buf.chpp + i] = 0;
		eraser_cursor_img->buf.pixels[2 * eraser_cursor_img->buf.chpp + i] = 0;
		eraser_cursor_img->buf.pixels[3 * eraser_cursor_img->buf.chpp + i] = 255;
	}
	eraser_cursor_img->gen_texture();
}

void Canvas::draw()
{
	fs_wget(*this, CHECKERBOARD).draw(CHECKERBOARD_TSLOT);
	fs_wget(*this, SPRITE).draw(CANVAS_SPRITE_TSLOT);
	if (binfo.show_preview)
		fs_wget(*this, BRUSH_PREVIEW).draw(BRUSH_PREVIEW_TSLOT);
	minor_gridlines.draw();
	major_gridlines.draw();
	if (cursor_in_canvas)
		draw_cursor();
}

void Canvas::draw_cursor()
{
	if (pipette_ready || binfo.tool & BrushTool::CAMERA
		|| (binfo.show_preview && binfo.tool & (BrushTool::LINE | BrushTool::FILL | BrushTool::RECT_FILL | BrushTool::RECT_OUTLINE)))
		return;
	switch (binfo.tip)
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

void Canvas::sync_sprite_with_image()
{
	if (image)
	{
		FlatSprite& sprite = fs_wget(*this, SPRITE);
		sprite.self.transform.scale = { image->buf.width, image->buf.height };
		sprite.update_transform();
	}
}

void Canvas::sync_checkerboard_with_image()
{
	if (image)
	{
		set_checkerboard_uv_size(0.5f * image->buf.width * checker_size_inv.x, 0.5f * image->buf.height * checker_size_inv.y);
		fs_wget(*this, CHECKERBOARD).self.transform.scale = { image->buf.width, image->buf.height };
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

		Buffer& ebuf = binfo.eraser_preview_image->buf;
		if (!ebuf.same_dimensions_as(image->buf))
		{
			delete[] ebuf.pixels;
			ebuf = image->buf;
			ebuf.width *= BrushInfo::eraser_preview_img_sx;
			ebuf.height *= BrushInfo::eraser_preview_img_sy;
			ebuf.pxnew();
		}
		static const Byte eraser_preview_arr[BrushInfo::eraser_preview_img_sx * BrushInfo::eraser_preview_img_sy * 4] = {
			255, 255, 255, 0  , 0  , 0  , 0  , 0  ,
			0  , 0  , 0  , 0  , 255, 255, 255, 0
		};
		for (Dim x = 0; x < image->buf.width; ++x)
			for (Dim y = 0; y < image->buf.height; ++y)
				for (Dim h = 0; h < BrushInfo::eraser_preview_img_sy; ++h)
					memcpy(ebuf.pos(BrushInfo::eraser_preview_img_sx * x, BrushInfo::eraser_preview_img_sy * y + h),
						eraser_preview_arr + h * BrushInfo::eraser_preview_img_sx * 4, BrushInfo::eraser_preview_img_sx * 4);
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
	sync_sprite_with_image();
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

void Canvas::update_brush_tool_and_tip()
{
	cursor_cancel();
	binfo.tool = MBrushes->get_brush_tool();
	binfo.tip = MBrushes->get_brush_tip();
	switch (binfo.tool)
	{
	case BrushTool::CAMERA:
		brush_under_tool_and_tip = &CBImpl::Camera::brush;
		break;
	case BrushTool::PAINT:
		if (binfo.tip & BrushTip::PENCIL)
			brush_under_tool_and_tip = &CBImpl::Paint::brush_pencil;
		else if (binfo.tip & BrushTip::PEN)
			brush_under_tool_and_tip = &CBImpl::Paint::brush_pen;
		else if (binfo.tip & BrushTip::ERASER)
			brush_under_tool_and_tip = &CBImpl::Paint::brush_eraser;
		else if (binfo.tip & BrushTip::SELECT)
			brush_under_tool_and_tip = &CBImpl::Paint::brush_select;
		break;
	case BrushTool::LINE:
		if (binfo.tip & BrushTip::PENCIL)
			brush_under_tool_and_tip = &CBImpl::Line::brush_pencil;
		else if (binfo.tip & BrushTip::PEN)
			brush_under_tool_and_tip = &CBImpl::Line::brush_pen;
		else if (binfo.tip & BrushTip::ERASER)
			brush_under_tool_and_tip = &CBImpl::Line::brush_eraser;
		else if (binfo.tip & BrushTip::SELECT)
			brush_under_tool_and_tip = &CBImpl::Line::brush_select;
		break;
	case BrushTool::RECT_FILL:
		if (binfo.tip & BrushTip::PENCIL)
			brush_under_tool_and_tip = &CBImpl::RectFill::brush_pencil;
		else if (binfo.tip & BrushTip::PEN)
			brush_under_tool_and_tip = &CBImpl::RectFill::brush_pen;
		else if (binfo.tip & BrushTip::ERASER)
			brush_under_tool_and_tip = &CBImpl::RectFill::brush_eraser;
		else if (binfo.tip & BrushTip::SELECT)
			brush_under_tool_and_tip = &CBImpl::RectFill::brush_select;
		break;
	}
	if (binfo.tip & (BrushTip::PENCIL | BrushTip::PEN))
		fs_wget(*this, BRUSH_PREVIEW).image = binfo.preview_image;
	else if (binfo.tip & BrushTip::ERASER)
		fs_wget(*this, BRUSH_PREVIEW).image = binfo.eraser_preview_image;
}

IPosition Canvas::brush_pos_under_cursor() const
{
	Position world_pos = MEasel->to_world_coordinates(Machine.cursor_screen_pos());
	Position local_pos = local_of(world_pos) + 0.5f * Position(image->buf.width, image->buf.height);
	return IPosition(local_pos);
}

bool Canvas::brush_pos_in_image_bounds(int x, int y) const
{
	return on_interval(x, 0, image->buf.width - 1) && on_interval(y, 0, image->buf.height - 1);
}

void Canvas::hover_pixel_under_cursor()
{
	IPosition pos = brush_pos_under_cursor();
	if (in_diagonal_rect(pos, {}, { image->buf.width - 1, image->buf.height - 1 }) || binfo.brushing)
	{
		cursor_in_canvas = true;
		if (pos != binfo.image_pos)
		{
			if (binfo.brushing && binfo.last_brush_pos != IPosition{ -1, -1 })
				brush_move_to(pos);
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
	IPosition pos = brush_pos_under_cursor();
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
	// LATER this is fine for when hovering over image, as it will replace the underlying pixel. However, the checkerboard will still show underneath, so the pen cursor should still blend with that.
	// Implement some kind of mechanism that will toggle between two modulations (maybe custom shader, using uniforms for performance): one that is blended with checker1, and the other with checker2.
	// Upon changing the checkerboard colors, this will need to be called again.
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
		{
			IPosition pos = brush_pos_under_cursor();
			brush(pos.x, pos.y);
		}
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
	if (brush_pos_in_image_bounds(x, y))
	{
		binfo.image_pos = { x, y };
		if (!binfo.brushing)
			brush_start(x, y);
		(*brush_under_tool_and_tip)(*this, x, y);
	}
}

void Canvas::brush_start(int x, int y)
{
	binfo.brushing = true;
	binfo.reset();
	binfo.image_pos = { x, y };
	binfo.starting_pos = { x, y };
	switch (binfo.tool)
	{
	case BrushTool::LINE:
		CBImpl::Line::start(*this);
		break;
	case BrushTool::RECT_FILL:
		if (binfo.tip & BrushTip::PENCIL)
			CBImpl::RectFill::start_pencil(*this);
		else if (binfo.tip & BrushTip::PEN)
			CBImpl::RectFill::start_pen(*this);
		else if (binfo.tip & BrushTip::ERASER)
			CBImpl::RectFill::start_eraser(*this);
		// LATER select
		break;
	}
}

void Canvas::brush_submit()
{
	if (binfo.brushing)
	{
		switch (binfo.tool)
		{
		case BrushTool::PAINT:
			CBImpl::Paint::brush_submit(*this);
			break;
		case BrushTool::LINE:
			if (binfo.tip & BrushTip::PENCIL)
				CBImpl::Line::submit_pencil(*this);
			else if (binfo.tip & BrushTip::PEN)
				CBImpl::Line::submit_pen(*this);
			else if (binfo.tip & BrushTip::ERASER)
				CBImpl::Line::submit_eraser(*this);
			// LATER select
			break;
		case BrushTool::RECT_FILL:
			if (binfo.tip & BrushTip::PENCIL)
				CBImpl::RectFill::submit_pencil(*this);
			else if (binfo.tip & BrushTip::PEN)
				CBImpl::RectFill::submit_pen(*this);
			else if (binfo.tip & BrushTip::ERASER)
				CBImpl::RectFill::submit_eraser(*this);
			// LATER select
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
	switch (binfo.tool)
	{
	case BrushTool::PAINT:
		auto& interp = binfo.interps.line;
		interp.start = binfo.image_pos;
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

void BrushInfo::reset()
{
	brushing_bbox.x1 = INT_MAX;
	brushing_bbox.x2 = INT_MIN;
	brushing_bbox.y1 = INT_MAX;
	brushing_bbox.y2 = INT_MIN;
	image_pos = { -1, -1 };
	last_brush_pos = { -1, -1 };
	starting_pos = { -1, -1 };
	if (show_preview)
	{
		if (tool & BrushTool::LINE)
		{
			if (tip & BrushTip::PENCIL)
				CBImpl::Line::reset_pencil(*this);
			else if (tip & BrushTip::PEN)
				CBImpl::Line::reset_pen(*this);
			else if (tip & BrushTip::ERASER)
				CBImpl::Line::reset_eraser(*this);
		}
		else if (tool & BrushTool::RECT_FILL)
		{
			if (tip & BrushTip::PENCIL)
				CBImpl::RectFill::reset_pencil(*this);
			else if (tip & BrushTip::PEN)
				CBImpl::RectFill::reset_pen(*this);
			else if (tip & BrushTip::ERASER)
				CBImpl::RectFill::reset_eraser(*this);
		}
	}
	show_preview = false;
	storage_1c.clear();
	storage_2c.clear();
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
		if (k.action == IAction::PRESS && k.key == Key::ESCAPE)
		{
			if (canvas().cursor_cancel())
				k.consumed = true;
		}
		else if (k.action == IAction::PRESS && k.key == Key::Z && (k.mods & Mods::CONTROL))
		{
			canvas().cursor_cancel(); // no consume
		}
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
		canvas().hover_pixel_under_cursor();
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
