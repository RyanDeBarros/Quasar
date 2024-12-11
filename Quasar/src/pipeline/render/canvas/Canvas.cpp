#include "Canvas.h"

#include <glm/gtc/type_ptr.hpp>

#include "CanvasBrushImpl.h"
#include "SelectionMants.h"
#include "../FlatSprite.h"
#include "ImplUtility.h"
#include "../Uniforms.h"
#include "user/Machine.h"
#include "../../panels/Easel.h"
#include "../../panels/BrushesPanel.h"
#include "../../panels/Palette.h"

constexpr GLuint CHECKERBOARD_TSLOT = 0;
constexpr GLuint CURSOR_ERASER_TSLOT = 1;
constexpr GLuint CURSOR_SELECT_TSLOT = 2;
constexpr GLuint BRUSH_PREVIEW_TSLOT = 3;
constexpr GLuint SELECTION_SUBIMAGE_TSLOT = 4;
constexpr GLuint CANVAS_SPRITE_TSLOT = 5;

Canvas::Canvas(Shader* cursor_shader)
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", std::to_string(GLC.max_texture_image_units) } }),
	Widget(_W_COUNT), brush_under_tool_and_tip(&CBImpl::Camera::brush), eraser_cursor_img(std::make_shared<Image>())
{
	binfo.canvas = this;
	binfo.preview_image = std::make_shared<Image>();
	binfo.eraser_preview_image = std::make_shared<Image>();
	binfo.selection_subimage = std::make_shared<Image>();
	initialize_widget(cursor_shader);
	initialize_dot_cursor();

	battr.tolerance.check_a = true;
	battr.tolerance.check_s_hsv = true;
	battr.tolerance.s_hsv1 = 0.0f;
	battr.tolerance.s_hsv2 = 0.5f;
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
	assign_widget(this, SELECTION, std::make_shared<SelectionMants>());
	binfo.smants = get<SelectionMants>(SELECTION);
	assign_widget(this, SELECTION_PREVIEW, std::make_shared<SelectionMants>());
	binfo.smants_preview = get<SelectionMants>(SELECTION_PREVIEW);
	binfo.smants_preview->speed *= -1;
	assign_widget(this, SELECTION_SUBIMAGE, std::make_shared<FlatSprite>(&sprite_shader));
	fs_wget(*this, SELECTION_SUBIMAGE).set_texture_slot(SELECTION_SUBIMAGE_TSLOT).image = binfo.selection_subimage;
	binfo.sel_subimg_sprite = get<FlatSprite>(SELECTION_SUBIMAGE);

	assign_widget(this, SPRITE, std::make_shared<FlatSprite>(&sprite_shader));
	fs_wget(*this, SPRITE).set_texture_slot(CANVAS_SPRITE_TSLOT);
}

void Canvas::initialize_dot_cursor()
{
	dot_cursor_buf.width = 10;
	dot_cursor_buf.height = 10;
	dot_cursor_buf.chpp = 4;
	dot_cursor_buf.pxnew();
	static const Byte dot_array[10 * 10] = {
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
	if (binfo.show_brush_preview)
		fs_wget(*this, BRUSH_PREVIEW).draw(BRUSH_PREVIEW_TSLOT);
	if (binfo.show_selection_subimage)
		fs_wget(*this, SELECTION_SUBIMAGE).draw(SELECTION_SUBIMAGE_TSLOT);
	if (binfo.show_selection_preview)
	{
		binfo.smants_preview->send_uniforms();
		binfo.smants_preview->draw();
		binfo.smants->send_uniforms();
	}
	minor_gridlines.draw();
	major_gridlines.draw();
	if (cursor_in_canvas)
		draw_cursor();
	binfo.smants->draw();
}

void Canvas::draw_cursor()
{
	if (binfo.tool & BrushTool::CAMERA
		|| (binfo.show_brush_preview && binfo.tool & (BrushTool::LINE | BrushTool::FILL | BrushTool::RECT_FILL | BrushTool::RECT_OUTLINE | BrushTool::ELLIPSE_OUTLINE | BrushTool::ELLIPSE_FILL)))
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
	Uniforms::send_matrix3(sprite_shader, "uVP", vp);
	Uniforms::send_matrix3(minor_gridlines.shader, "uVP", vp);
	Uniforms::send_matrix3(major_gridlines.shader, "uVP", vp);
	binfo.smants->send_screen_size(MainWindow->size());
	binfo.smants_preview->send_screen_size(MainWindow->size());
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

void Canvas::process()
{
	if (MEasel->cursor_in_clipping())
		hover_pixel_under_cursor();
	else
		unhover();
	binfo.smants->send_time(Machine.time());
	binfo.smants_preview->send_time(Machine.time());
	if (move_selection_info.moving)
		process_move_selection();
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

		Buffer& sbuf = binfo.selection_subimage->buf;
		if (!sbuf.same_dimensions_as(image->buf))
		{
			delete[] sbuf.pixels;
			sbuf = image->buf;
			sbuf.pxnew();
		}
		memset(sbuf.pixels, 0, sbuf.bytes());
		binfo.selection_subimage->gen_texture();
		binfo.selection_subimage->resend_texture();

		fs_wget(*this, BRUSH_PREVIEW).self.transform.scale = { image->buf.width, image->buf.height };
		fs_wget(*this, SELECTION_SUBIMAGE).self.transform.scale = { image->buf.width, image->buf.height };
	}
}

void Canvas::sync_smants_with_image()
{
	if (image)
	{
		binfo.smants->set_size(image->buf.width, image->buf.height);
		binfo.smants_preview->set_size(image->buf.width, image->buf.height);
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
	fs_wget(*this, SELECTION_SUBIMAGE).update_transform().ur->send_buffer();
	fs_wget(*this, SPRITE).update_transform().ur->send_buffer();
	binfo.smants->send_vp(MEasel->vp * self.matrix());
	binfo.smants_preview->send_vp(MEasel->vp * self.matrix());
	sync_cursor_with_widget();
}

void Canvas::sync_gfx_with_image()
{
	sync_sprite_with_image();
	sync_checkerboard_with_image();
	sync_brush_preview_with_image();
	sync_smants_with_image();
	sync_gridlines_with_image();
	sync_transform();
	full_brush_reset();
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
	auto new_tool = MBrushes->get_brush_tool();
	auto new_tip = MBrushes->get_brush_tip();
	// TODO instead of different bool vars show_selection_subimage, brushing, etc., use one enum status.
	if (binfo.show_selection_subimage)
		CBImpl::transition_selection_tip(*this, binfo.tip, new_tip);
	else if (binfo.tool != new_tool || binfo.tip != new_tip) // LATER add support for switching tool/tip mid brush?
		cursor_cancel();
	binfo.tool = new_tool;
	binfo.tip = new_tip;
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
	case BrushTool::RECT_OUTLINE:
		if (binfo.tip & BrushTip::PENCIL)
			brush_under_tool_and_tip = &CBImpl::RectOutline::brush_pencil;
		else if (binfo.tip & BrushTip::PEN)
			brush_under_tool_and_tip = &CBImpl::RectOutline::brush_pen;
		else if (binfo.tip & BrushTip::ERASER)
			brush_under_tool_and_tip = &CBImpl::RectOutline::brush_eraser;
		else if (binfo.tip & BrushTip::SELECT)
			brush_under_tool_and_tip = &CBImpl::RectOutline::brush_select;
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
	case BrushTool::ELLIPSE_OUTLINE:
		if (binfo.tip & BrushTip::PENCIL)
			brush_under_tool_and_tip = &CBImpl::EllipseOutline::brush_pencil;
		else if (binfo.tip & BrushTip::PEN)
			brush_under_tool_and_tip = &CBImpl::EllipseOutline::brush_pen;
		else if (binfo.tip & BrushTip::ERASER)
			brush_under_tool_and_tip = &CBImpl::EllipseOutline::brush_eraser;
		else if (binfo.tip & BrushTip::SELECT)
			brush_under_tool_and_tip = &CBImpl::EllipseOutline::brush_select;
		break;
	case BrushTool::ELLIPSE_FILL:
		if (binfo.tip & BrushTip::PENCIL)
			brush_under_tool_and_tip = &CBImpl::EllipseFill::brush_pencil;
		else if (binfo.tip & BrushTip::PEN)
			brush_under_tool_and_tip = &CBImpl::EllipseFill::brush_pen;
		else if (binfo.tip & BrushTip::ERASER)
			brush_under_tool_and_tip = &CBImpl::EllipseFill::brush_eraser;
		else if (binfo.tip & BrushTip::SELECT)
			brush_under_tool_and_tip = &CBImpl::EllipseFill::brush_select;
		break;
	case BrushTool::FILL:
		if (binfo.tip & BrushTip::PENCIL)
			brush_under_tool_and_tip = &CBImpl::BucketFill::brush_pencil;
		else if (binfo.tip & BrushTip::PEN)
			brush_under_tool_and_tip = &CBImpl::BucketFill::brush_pen;
		else if (binfo.tip & BrushTip::ERASER)
			brush_under_tool_and_tip = &CBImpl::BucketFill::brush_eraser;
		else if (binfo.tip & BrushTip::SELECT)
			brush_under_tool_and_tip = &CBImpl::BucketFill::brush_select;
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
	return IPosition(floori(local_pos.x), floori(local_pos.y));
}

bool Canvas::brush_pos_in_image_bounds(int x, int y) const
{
	return on_interval(x, 0, image->buf.width - 1) && on_interval(y, 0, image->buf.height - 1);
}

void Canvas::hover_pixel_under_cursor()
{
	IPosition pos = brush_pos_under_cursor();
	if (brush_pos_in_image_bounds(pos.x, pos.y))
	{
		cursor_in_canvas = true;
		if (pos != binfo.image_pos)
		{
			if (cursor_state != CursorState::UP)
				brush_move_to(pos);
			binfo.image_pos = pos;
			hover_pixel_at(pixel_position(binfo.image_pos));
		}
		if (MainWindow->is_alt_pressed())
		{
			MainWindow->release_cursor(&dot_cursor_wh);
			MainWindow->request_cursor(&pipette_cursor_wh, Machine.cursors.CROSSHAIR);
			using_pipette = true;
		}
		else
		{
			MainWindow->release_cursor(&pipette_cursor_wh);
			MainWindow->request_cursor(&dot_cursor_wh, dot_cursor);
			using_pipette = false;
		}
	}
	else
	{
		cursor_in_canvas = false;
		if (binfo.brushing && pos != binfo.image_pos)
			brush_move_to(pos);
		binfo.image_pos = pos;
		unhover();
	}
}

void Canvas::unhover()
{
	MainWindow->release_cursor(&dot_cursor_wh);
	MainWindow->release_cursor(&pipette_cursor_wh);
	cursor_in_canvas = false;
}

void Canvas::hover_pixel_at(Position pos)
{
	Position& existing = wp_at(CURSOR_PENCIL).transform.position;
	if (existing != pos)
	{
		existing = pos;
		wp_at(CURSOR_PEN).transform.position = pos;
		wp_at(CURSOR_ERASER).transform.position = pos;
		wp_at(CURSOR_SELECT).transform.position = pos;
		sync_cursor_with_widget();
	}
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

PixelRGBA Canvas::applied_pencil_rgba() const
{
	return cursor_state == Canvas::CursorState::DOWN_PRIMARY ? pric_pxs : altc_pxs;
}

PixelRGBA Canvas::applied_pen_rgba() const
{
	return cursor_state == Canvas::CursorState::DOWN_PRIMARY ? pric_pen_pxs : altc_pen_pxs;
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
	if (using_pipette)
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
		bool begin = false;
		if (button == MouseButton::LEFT && cursor_state != CursorState::DOWN_ALTERNATE)
		{
			cursor_state = CursorState::DOWN_PRIMARY;
			begin = true;
		}
		else if (button == MouseButton::RIGHT && cursor_state != CursorState::DOWN_PRIMARY)
		{
			cursor_state = CursorState::DOWN_ALTERNATE;
			begin = true;
		}
		if (begin && cursor_in_canvas)
		{
			IPosition pos = brush_pos_under_cursor();
			brush(pos.x, pos.y);
		}
	}
}

bool Canvas::cursor_enter()
{
	if (binfo.brushing)
	{
		brush_submit();
		cursor_state = CursorState::UP;
		set_cursor_color(primary_color);
		return true;
	}
	else if (binfo.show_selection_subimage)
	{
		apply_selection();
		return true;
	}
	return false;
}

void Canvas::cursor_release(MouseButton button)
{
	if ((button == MouseButton::LEFT && cursor_state == CursorState::DOWN_PRIMARY) || (button == MouseButton::RIGHT && cursor_state == CursorState::DOWN_ALTERNATE))
		cursor_enter();
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
	else if (binfo.show_selection_subimage)
		apply_selection();
	return false;
}

void Canvas::full_brush_reset()
{
	deselect_all(); // this internally calls cursor_cancel()
}

void Canvas::brush(int x, int y)
{
	if (!move_selection_info.moving && brush_pos_in_image_bounds(x, y))
	{
		if (!binfo.brushing)
		{
			apply_selection();
			brush_start(x, y);
		}
		(*brush_under_tool_and_tip)(*this, x, y);
	}
}

void Canvas::brush_start(int x, int y)
{
	set_cursor_color(applied_color());
	binfo.brushing = true;
	binfo.reset();
	binfo.last_brush_pos = { x, y };
	binfo.starting_pos = { x, y };
	if (binfo.tool & BrushTool::PAINT && binfo.tip & BrushTip::SELECT)
		CBImpl::Paint::start_select(*this);
	else if (binfo.tool & BrushTool::LINE)
	{
		if (binfo.tip & BrushTip::SELECT)
			CBImpl::Line::start_select(*this);
		else
			CBImpl::Line::start(*this);
	}
	else if (binfo.tool & BrushTool::RECT_OUTLINE)
	{
		if (binfo.tip & BrushTip::SELECT)
			CBImpl::RectOutline::start_select(*this);
		else
			CBImpl::RectOutline::start(*this);
	}
	else if (binfo.tool & BrushTool::RECT_FILL)
	{
		if (binfo.tip & BrushTip::SELECT)
			CBImpl::RectFill::start_select(*this);
		else
			CBImpl::RectFill::start(*this);
	}
	else if (binfo.tool & BrushTool::ELLIPSE_OUTLINE)
	{
		if (binfo.tip & BrushTip::SELECT)
			CBImpl::EllipseOutline::start_select(*this);
		else
			CBImpl::EllipseOutline::start(*this);
	}
	else if (binfo.tool & BrushTool::ELLIPSE_FILL)
	{
		if (binfo.tip & BrushTip::SELECT)
			CBImpl::EllipseFill::start_select(*this);
		else
			CBImpl::EllipseFill::start(*this);
	}
}

void Canvas::brush_submit()
{
	switch (binfo.tool)
	{
	case BrushTool::PAINT:
		if (binfo.tip & BrushTip::PENCIL)
			CBImpl::Paint::submit_pencil(*this);
		else if (binfo.tip & BrushTip::PEN)
			CBImpl::Paint::submit_pen(*this);
		else if (binfo.tip & BrushTip::ERASER)
			CBImpl::Paint::submit_eraser(*this);
		else if (binfo.tip & BrushTip::SELECT)
			CBImpl::Paint::submit_select(*this);
		break;
	case BrushTool::LINE:
		if (binfo.tip & BrushTip::PENCIL)
			CBImpl::Line::submit_pencil(*this);
		else if (binfo.tip & BrushTip::PEN)
			CBImpl::Line::submit_pen(*this);
		else if (binfo.tip & BrushTip::ERASER)
			CBImpl::Line::submit_eraser(*this);
		else if (binfo.tip & BrushTip::SELECT)
			CBImpl::Line::submit_select(*this);
		break;
	case BrushTool::RECT_OUTLINE:
		if (binfo.tip & BrushTip::PENCIL)
			CBImpl::RectOutline::submit_pencil(*this);
		else if (binfo.tip & BrushTip::PEN)
			CBImpl::RectOutline::submit_pen(*this);
		else if (binfo.tip & BrushTip::ERASER)
			CBImpl::RectOutline::submit_eraser(*this);
		else if (binfo.tip & BrushTip::SELECT)
			CBImpl::RectOutline::submit_select(*this);
		break;
	case BrushTool::RECT_FILL:
		if (binfo.tip & BrushTip::PENCIL)
			CBImpl::RectFill::submit_pencil(*this);
		else if (binfo.tip & BrushTip::PEN)
			CBImpl::RectFill::submit_pen(*this);
		else if (binfo.tip & BrushTip::ERASER)
			CBImpl::RectFill::submit_eraser(*this);
		else if (binfo.tip & BrushTip::SELECT)
			CBImpl::RectFill::submit_select(*this);
		break;
	case BrushTool::ELLIPSE_OUTLINE:
		if (binfo.tip & BrushTip::PENCIL)
			CBImpl::EllipseOutline::submit_pencil(*this);
		else if (binfo.tip & BrushTip::PEN)
			CBImpl::EllipseOutline::submit_pen(*this);
		else if (binfo.tip & BrushTip::ERASER)
			CBImpl::EllipseOutline::submit_eraser(*this);
		else if (binfo.tip & BrushTip::SELECT)
			CBImpl::EllipseOutline::submit_select(*this);
		break;
	case BrushTool::ELLIPSE_FILL:
		if (binfo.tip & BrushTip::PENCIL)
			CBImpl::EllipseFill::submit_pencil(*this);
		else if (binfo.tip & BrushTip::PEN)
			CBImpl::EllipseFill::submit_pen(*this);
		else if (binfo.tip & BrushTip::ERASER)
			CBImpl::EllipseFill::submit_eraser(*this);
		else if (binfo.tip & BrushTip::SELECT)
			CBImpl::EllipseFill::submit_select(*this);
		break;
	}
	binfo.brushing = false;
	binfo.reset();
}

void Canvas::brush_cancel()
{
	binfo.brushing = false;
	binfo.cancelling = true;
	binfo.reset();
}

void Canvas::brush_move_to(IPosition pos)
{
	if (!(binfo.tool & BrushTool::CAMERA))
		CBImpl::brush_move_to(*this, pos.x, pos.y);
}

void Canvas::select_all()
{
	cursor_cancel();
	if (image)
	{
		binfo.brushing_bbox = IntBounds::NADIR;
		for (int x = 0; x < image->buf.width; ++x)
			for (int y = 0; y < image->buf.height; ++y)
				if (binfo.add_to_selection({ x, y }))
					update_bbox(binfo.brushing_bbox, x, y);
		binfo.smants->send_buffer(binfo.brushing_bbox);
		binfo.push_selection_to_history();
	}
}

void Canvas::deselect_all()
{
	cursor_cancel();
	if (image)
	{
		if (binfo.show_selection_subimage)
			apply_selection();
		else
		{
			binfo.brushing_bbox = binfo.clear_selection();
			binfo.smants->send_buffer(binfo.brushing_bbox);
			binfo.push_selection_to_history();
		}
	}
}

static bool selection_interaction_disabled(BrushInfo& binfo)
{
	return binfo.smants->get_points().empty() || binfo.brushing;
}

void Canvas::fill_selection_primary()
{
	if (selection_interaction_disabled(binfo))
		return;
	if (binfo.tip & BrushTip::PENCIL)
		CBImpl::fill_selection_pencil(*this, pric_pxs);
	else if (binfo.tip & BrushTip::PEN)
		CBImpl::fill_selection_pen(*this, pric_pxs);
	else if (binfo.tip & BrushTip::ERASER)
		CBImpl::fill_selection_eraser(*this);
}

void Canvas::fill_selection_alternate()
{
	if (selection_interaction_disabled(binfo))
		return;
	if (binfo.tip & BrushTip::PENCIL)
		CBImpl::fill_selection_pencil(*this, altc_pxs);
	else if (binfo.tip & BrushTip::PEN)
		CBImpl::fill_selection_pen(*this, altc_pxs);
	else if (binfo.tip & BrushTip::ERASER)
		CBImpl::fill_selection_eraser(*this);
}

bool Canvas::delete_selection()
{
	if (selection_interaction_disabled(binfo))
		return false;
	CBImpl::fill_selection_eraser(*this);
	return true;
}

void Canvas::process_move_selection()
{
	move_selection_info.held_time += move_selection_info.held_speed_factor * Machine.delta_time();
	if (move_selection_info.on_starting_interval)
	{
		if (move_selection_info.held_time > move_selection_info.held_start_interval)
		{
			move_selection_info.held_time -= move_selection_info.held_start_interval;
			while (move_selection_info.held_time > move_selection_info.held_interval)
				move_selection_info.held_time -= move_selection_info.held_interval;
			move_selection(move_selection_info.move_x, move_selection_info.move_y);
			move_selection_info.on_starting_interval = false;
		}
	}
	else if (move_selection_info.held_time > move_selection_info.held_interval)
	{

		do { move_selection_info.held_time -= move_selection_info.held_interval; } while (move_selection_info.held_time > move_selection_info.held_interval);
		move_selection(move_selection_info.move_x, move_selection_info.move_y);
	}
}

bool Canvas::start_move_selection(int dx, int dy)
{
	if (!move_selection(dx, dy))
		return false;
	move_selection_info.moving = true;
	move_selection_info.on_starting_interval = true;
	move_selection_info.held_time = 0.0f;
	move_selection_info.move_x = dx;
	move_selection_info.move_y = dy;
	return true;
}

bool Canvas::move_selection(int dx, int dy)
{
	if (selection_interaction_disabled(binfo) || (dx == 0 && dy == 0))
		return false;
	if (binfo.tip & BrushTip::PENCIL)
	{
		CBImpl::move_selection_with_pixels_pencil(*this, dx, dy);
		return true;
	}
	else if (binfo.tip & BrushTip::PEN)
	{
		CBImpl::move_selection_with_pixels_pen(*this, dx, dy);
		return true;
	}
	else if (binfo.tip & (BrushTip::ERASER | BrushTip::SELECT))
	{
		CBImpl::move_selection_without_pixels(*this, dx, dy);
		return true;
	}
	else
		return false;
}

void Canvas::apply_selection()
{
	if (binfo.show_selection_subimage)
	{
		if (binfo.tip & BrushTip::PENCIL)
			CBImpl::apply_selection_pencil(*this);
		else if (binfo.tip & BrushTip::PEN)
			CBImpl::apply_selection_pen(*this);
	}
}
