#include "BrushesPanel.h"

#include <glm/gtc/type_ptr.hpp>

#include <ImplUtility.h>
#include "edit/color/Color.h"
#include "../render/Uniforms.h"
#include "variety/GLutility.h"
#include "user/Machine.h"
#include "Easel.h"
#include "../render/canvas/Canvas.h"

// ---------- LAYOUT ----------

const float tip_w_pencil = 60;
const float tip_w_pen = 40;
const float tip_w_eraser = 65;
const float tip_w_select = 65;
const float starting_tip_h = 25;
const float tip_full_w = tip_w_pencil + tip_w_pen + tip_w_eraser + tip_w_select;
const float starting_tip_x = -0.5f * tip_full_w;
const float starting_tip_y = -10;

const float tool_btn_side = 100;
const float tool_btn_spacing = 10;

BrushesPanel::BrushesPanel()
	: bkg_shader(FileSystem::shader_path("color_square.vert"), FileSystem::shader_path("color_square.frag")),
	round_rect_shader(FileSystem::shader_path("round_rect.vert"), FileSystem::shader_path("round_rect.frag")), widget(_W_COUNT)
{
	connect_input_handlers();
}

void BrushesPanel::initialize()
{
	initialize_widget();
}

void BrushesPanel::initialize_widget()
{
	assign_widget(&widget, BACKGROUND, std::make_shared<W_UnitRenderable>(&bkg_shader));
	ur_wget(widget, BACKGROUND).set_attribute(1, glm::value_ptr(RGBA(0.2f, 0.1f, 0.3f, 0.1f).as_vec())).send_buffer();

	assign_widget(&widget, EMPTY_BUTTON_TIP, std::make_shared<Widget>(_W_ETIP_COUNT));
	assign_widget(&widget, EMPTY_BUTTON_TOOL, std::make_shared<Widget>(_W_ETOOL_COUNT));

	Widget* tip_parent = widget.children[EMPTY_BUTTON_TIP].get();
	tip_parent->self.pivot.y = 1;

	ToggleTButtonArgs tba(&mb_handler, &round_rect_shader, &vp);
	tba.pivot = { 0, 1 };
	tba.frange = Fonts::label_black;
	tba.font_size = 16;

	tba.text = "PENCIL";
	tba.transform = { { starting_tip_x, starting_tip_y }, { tip_w_pencil, starting_tip_h } };
	tba.on_select = fconv_on_action([this]() { select_brush_tip(BrushTip::PENCIL); });
	assign_widget(tip_parent, BUTTON_TIP_PENCIL, std::make_shared<ToggleTButton>(tba));
	
	tba.text = "PEN";
	tba.transform.position.x += tba.transform.scale.x;
	tba.transform.scale.x = tip_w_pen;
	tba.on_select = fconv_on_action([this]() { select_brush_tip(BrushTip::PEN); });
	assign_widget(tip_parent, BUTTON_TIP_PEN, std::make_shared<ToggleTButton>(tba));

	tba.text = "ERASER";
	tba.transform.position.x += tba.transform.scale.x;
	tba.transform.scale.x = tip_w_eraser;
	tba.on_select = fconv_on_action([this]() { select_brush_tip(BrushTip::ERASER); });
	assign_widget(tip_parent, BUTTON_TIP_ERASER, std::make_shared<ToggleTButton>(tba));

	tba.text = "SELECT";
	tba.transform.position.x += tba.transform.scale.x;
	tba.transform.scale.x = tip_w_select;
	tba.on_select = fconv_on_action([this]() { select_brush_tip(BrushTip::SELECT); });
	assign_widget(tip_parent, BUTTON_TIP_SELECT, std::make_shared<ToggleTButton>(tba));

	toggle_group_tips.init({
		{ BUTTON_TIP_PENCIL, &tb_t_wget(*tip_parent, BUTTON_TIP_PENCIL) },
		{ BUTTON_TIP_PEN, &tb_t_wget(*tip_parent, BUTTON_TIP_PEN) },
		{ BUTTON_TIP_ERASER, &tb_t_wget(*tip_parent, BUTTON_TIP_ERASER) },
		{ BUTTON_TIP_SELECT, &tb_t_wget(*tip_parent, BUTTON_TIP_SELECT) },
		}, BUTTON_TIP_PENCIL);

	Widget* tool_parent = widget.children[EMPTY_BUTTON_TOOL].get();
	tool_parent->self.transform.position = { 0, starting_tip_y };
	tool_parent->self.pivot = { 0.5f, 0.5f };

	tba.pivot = { 0.5f, 0.5f };
	tba.font_size = 18;
	tba.transform.scale = { tool_btn_side, tool_btn_side };
	tba.transform.position.y = 1.5f * (tool_btn_side + tool_btn_spacing);
	
	tba.text = "CAMERA";
	tba.transform.position.x = -0.5f * (tool_btn_side + tool_btn_spacing);
	tba.on_select = fconv_on_action([this]() { select_brush_tool(BrushTool::CAMERA); });
	assign_widget(tool_parent, BUTTON_TOOL_CAMERA, std::make_shared<ToggleTButton>(tba));
	
	tba.text = "PAINT";
	tba.transform.position.x = 0.5f * (tool_btn_side + tool_btn_spacing);
	tba.on_select = fconv_on_action([this]() { select_brush_tool(BrushTool::PAINT); });
	assign_widget(tool_parent, BUTTON_TOOL_PAINT, std::make_shared<ToggleTButton>(tba));

	tba.transform.position.y -= tool_btn_side + tool_btn_spacing;
	tba.text = "LINE";
	tba.transform.position.x = -0.5f * (tool_btn_side + tool_btn_spacing);
	tba.on_select = fconv_on_action([this]() { select_brush_tool(BrushTool::LINE); });
	assign_widget(tool_parent, BUTTON_TOOL_LINE, std::make_shared<ToggleTButton>(tba));

	tba.text = "FILL";
	tba.transform.position.x = 0.5f * (tool_btn_side + tool_btn_spacing);
	tba.on_select = fconv_on_action([this]() { select_brush_tool(BrushTool::FILL); });
	assign_widget(tool_parent, BUTTON_TOOL_FILL, std::make_shared<ToggleTButton>(tba));

	tba.transform.position.y -= tool_btn_side + tool_btn_spacing;
	tba.text = "RECT\nOUTLINE";
	tba.transform.position.x = -0.5f * (tool_btn_side + tool_btn_spacing);
	tba.on_select = fconv_on_action([this]() { select_brush_tool(BrushTool::RECT_OUTLINE); });
	assign_widget(tool_parent, BUTTON_TOOL_RECT_OUTLINE, std::make_shared<ToggleTButton>(tba));

	tba.text = "RECT\nFILL";
	tba.transform.position.x = 0.5f * (tool_btn_side + tool_btn_spacing);
	tba.on_select = fconv_on_action([this]() { select_brush_tool(BrushTool::RECT_FILL); });
	assign_widget(tool_parent, BUTTON_TOOL_RECT_FILL, std::make_shared<ToggleTButton>(tba));

	tba.transform.position.y -= tool_btn_side + tool_btn_spacing;
	tba.text = "ELLIPSE\nOUTLINE";
	tba.transform.position.x = -0.5f * (tool_btn_side + tool_btn_spacing);
	tba.on_select = fconv_on_action([this]() { select_brush_tool(BrushTool::ELLIPSE_OUTLINE); });
	assign_widget(tool_parent, BUTTON_TOOL_ELLIPSE_OUTLINE, std::make_shared<ToggleTButton>(tba));

	tba.text = "ELLIPSE\nFILL";
	tba.transform.position.x = 0.5f * (tool_btn_side + tool_btn_spacing);
	tba.on_select = fconv_on_action([this]() { select_brush_tool(BrushTool::ELLIPSE_FILL); });
	assign_widget(tool_parent, BUTTON_TOOL_ELLIPSE_FILL, std::make_shared<ToggleTButton>(tba));

	toggle_group_tools.init({
		{ BUTTON_TOOL_CAMERA, &tb_t_wget(*tool_parent, BUTTON_TOOL_CAMERA) },
		{ BUTTON_TOOL_PAINT, &tb_t_wget(*tool_parent, BUTTON_TOOL_PAINT) },
		{ BUTTON_TOOL_LINE, &tb_t_wget(*tool_parent, BUTTON_TOOL_LINE) },
		{ BUTTON_TOOL_FILL, &tb_t_wget(*tool_parent, BUTTON_TOOL_FILL) },
		{ BUTTON_TOOL_RECT_OUTLINE, &tb_t_wget(*tool_parent, BUTTON_TOOL_RECT_OUTLINE) },
		{ BUTTON_TOOL_RECT_FILL, &tb_t_wget(*tool_parent, BUTTON_TOOL_RECT_FILL) },
		{ BUTTON_TOOL_ELLIPSE_OUTLINE, &tb_t_wget(*tool_parent, BUTTON_TOOL_ELLIPSE_OUTLINE) },
		{ BUTTON_TOOL_ELLIPSE_FILL, &tb_t_wget(*tool_parent, BUTTON_TOOL_ELLIPSE_FILL) },
		}, BUTTON_TOOL_CAMERA);
}

void BrushesPanel::connect_input_handlers()
{
	mb_handler.callback = [](const MouseButtonEvent&) {};
	key_handler.callback = [this](const KeyEvent& k) {
		if (k.action == IAction::PRESS && !(k.mods & (Mods::CONTROL | Mods::ALT | Mods::SUPER)))
		{
			if (!(k.mods & Mods::SHIFT))
			{
				switch (k.key)
				{
				case Key::ROW1:
					k.consumed = true;
					select_brush_tip(BrushTip::PENCIL);
					break;
				case Key::ROW2:
					k.consumed = true;
					select_brush_tip(BrushTip::PEN);
					break;
				case Key::ROW3:
					k.consumed = true;
					select_brush_tip(BrushTip::ERASER);
					break;
				case Key::ROW4:
					k.consumed = true;
					select_brush_tip(BrushTip::SELECT);
					break;
				case Key::E:
					k.consumed = true;
					select_brush_tool(BrushTool::ELLIPSE_OUTLINE);
					break;
				case Key::F:
					k.consumed = true;
					select_brush_tool(BrushTool::FILL);
					break;
				case Key::L:
					k.consumed = true;
					select_brush_tool(BrushTool::LINE);
					break;
				case Key::P:
					k.consumed = true;
					select_brush_tool(BrushTool::PAINT);
					break;
				case Key::R:
					k.consumed = true;
					select_brush_tool(BrushTool::RECT_OUTLINE);
					break;
				case Key::M:
					k.consumed = true;
					select_brush_tool(BrushTool::CAMERA);
					break;
				}
			}
			else
			{
				switch (k.key)
				{
				case Key::E:
					k.consumed = true;
					select_brush_tool(BrushTool::ELLIPSE_FILL);
					break;
				case Key::R:
					k.consumed = true;
					select_brush_tool(BrushTool::RECT_FILL);
					break;
				}
			}
		}
		};
}

void BrushesPanel::draw()
{
	ur_wget(widget, BACKGROUND).draw();
	toggle_group_tips.draw();
	toggle_group_tools.draw();
}

void BrushesPanel::_send_view()
{
	vp = vp_matrix();
	Uniforms::send_matrix3(bkg_shader, "uVP", vp);
	Uniforms::send_matrix3(round_rect_shader, "uVP", vp);
	unbind_shader();
	sync_widget();
}

Scale BrushesPanel::minimum_screen_display() const
{
	return to_screen_size({ std::max(tip_full_w, 2 * tool_btn_side + tool_btn_spacing) + 20,
		2 * (-starting_tip_y + starting_tip_h) + 4 * tool_btn_side + 3 * tool_btn_spacing});
}

void BrushesPanel::sync_widget()
{
	Scale app_size = get_app_size();
	w_tip_parent().self.transform.position.y = 0.5f * app_size.y;
	widget.wp_at(BACKGROUND).transform.scale = app_size;
	sync_ur(BACKGROUND);

	float sc = widget.scale1d();
	bool adapt_to_scale = cached_scale1d != sc;
	if (adapt_to_scale)
	{
		cached_scale1d = sc;
		toggle_group_tips.adapt_to_scale(sc);
	}
	toggle_group_tips.send_vp();

	if (adapt_to_scale)
		toggle_group_tools.adapt_to_scale(sc);
	toggle_group_tools.send_vp();
}

void BrushesPanel::sync_ur(size_t subw)
{
	Utils::set_vertex_pos_attributes(ur_wget(widget, subw), widget.wp_at(subw).relative_to(widget.self.transform));
}

void BrushesPanel::select_brush_tip(BrushTip tip)
{
	brush_tip = tip;
	toggle_group_tips.select(tip == BrushTip::PENCIL ? BUTTON_TIP_PENCIL
		: tip == BrushTip::PEN ? BUTTON_TIP_PEN
		: tip == BrushTip::ERASER ? BUTTON_TIP_ERASER
		: BUTTON_TIP_SELECT
	);
	MEasel->canvas().update_brush_tool_and_tip();
}

void BrushesPanel::select_brush_tool(BrushTool tool)
{
	brush_tool = tool;
	toggle_group_tools.select(tool == BrushTool::CAMERA ? BUTTON_TOOL_CAMERA
		: tool == BrushTool::PAINT ? BUTTON_TOOL_PAINT
		: tool == BrushTool::LINE ? BUTTON_TOOL_LINE
		: tool == BrushTool::RECT_OUTLINE ? BUTTON_TOOL_RECT_OUTLINE
		: tool == BrushTool::RECT_FILL ? BUTTON_TOOL_RECT_FILL
		: tool == BrushTool::ELLIPSE_OUTLINE ? BUTTON_TOOL_ELLIPSE_OUTLINE
		: tool == BrushTool::ELLIPSE_FILL ? BUTTON_TOOL_ELLIPSE_FILL
		: BUTTON_TOOL_FILL
	);
	MEasel->canvas().update_brush_tool_and_tip();
}

void BrushesPanel::process()
{
	if (cursor_in_clipping())
	{
		toggle_group_tips.process();
		toggle_group_tools.process();
	}
	else
	{
		toggle_group_tips.unhover();
		toggle_group_tools.unhover();
	}
}

Widget& BrushesPanel::w_tip_parent()
{
	return *widget.children[EMPTY_BUTTON_TIP];
}

Widget& BrushesPanel::w_tool_parent()
{
	return *widget.children[EMPTY_BUTTON_TOOL];
}
