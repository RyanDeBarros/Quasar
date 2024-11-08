#include "Button.h"

#include "user/Machine.h"

void Button::init(const WidgetPlacement& wp, TextRender* txt, RoundRect* bkg)
{
	children.resize(_W_COUNT);
	assign_widget(this, TEXT, txt);
	assign_widget(this, BKG, bkg);
	if (parent_mbh)
		parent_mbh->children.push_back(&handler);
	handler.callback = [this](const MouseButtonEvent& m) {
		if (m.action == IAction::PRESS)
		{
			Position local_cursor_pos;
			if (is_hovered(&local_cursor_pos))
			{
				on_press(m, local_cursor_pos);
				m.consumed = true;
				if (m.button == MouseButton::LEFT)
					left_pressed = true;
				else if (m.button == MouseButton::MIDDLE)
					middle_pressed = true;
				else if (m.button == MouseButton::RIGHT)
					right_pressed = true;
			}
		}
		else if (m.action == IAction::RELEASE)
		{
			Position local_cursor_pos;
			if (is_hovered(&local_cursor_pos))
			{
				if (is_pressed(m.button))
					m.consumed = true;
				on_release(m, local_cursor_pos);
			}
			if (m.button == MouseButton::LEFT)
				left_pressed = false;
			else if (m.button == MouseButton::MIDDLE)
				middle_pressed = false;
			else if (m.button == MouseButton::RIGHT)
				right_pressed = false;
		}
		};
	self = wp;
	text().format.horizontal_align = TextRender::HorizontalAlign::CENTER;
	text().format.vertical_align = TextRender::VerticalAlign::MIDDLE;
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler* parent_mbh, const UTF::String& txt)
	: bkg_shader(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(font, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(txt);
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler* parent_mbh, UTF::String&& txt)
	: bkg_shader(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(font, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(std::move(txt));
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler* parent_mbh, const UTF::String& txt)
	: bkg_shader(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(frange, font_size, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(txt);
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler* parent_mbh, UTF::String&& txt)
	: bkg_shader(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(frange, font_size, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(std::move(txt));
}

Button::~Button()
{
	if (parent_mbh)
		parent_mbh->remove_child(&handler);
}

void Button::draw() const
{
	bkg().draw();
	text().draw();
}

void Button::send_vp() const
{
	bkg().update_transform().send_buffer();
	text().send_vp(*vp);
}

void Button::process()
{
	if (is_hovered())
	{
		if (!hovering)
		{
			hovering = true;
			on_hover_enter();
		}
	}
	else
	{
		if (hovering)
		{
			hovering = false;
			on_hover_exit();
		}
	}
}

bool Button::is_pressed(MouseButton mb) const
{
	if (mb == MouseButton::LEFT)
		return left_pressed;
	else if (mb == MouseButton::MIDDLE)
		return middle_pressed;
	else if (mb == MouseButton::RIGHT)
		return right_pressed;
	else
		return false;
}

bool Button::is_hovered(Position* local_pos) const
{
	Position pos = bkg().local_of(Machine.cursor_world_coordinates(glm::inverse(*vp)));
	if (local_pos)
		*local_pos = pos;
	return on_interval(pos.x, -0.5f, 0.5f) && on_interval(pos.y, -0.5f, 0.5f);
}

StandardButton::StandardButton(const StandardButtonArgs& args)
	: Button(args.vp, { args.transform }, args.frange, args.font_size, args.rr_shader, args.mb_parent, args.text), normal_fill(args.normal_fill)
{
	prev_fill = &normal_fill;
	bkg().self.transform.scale = args.bkg_size;
	bkg().thickness = args.thickness;
	bkg().corner_radius = args.corner_radius;
	bkg().border_color = args.border_color;
	bkg().fill_color = args.normal_fill;
	bkg().update_all();
	// LATER cancel highlighting/cursor after clicking on button.
	on_hover_enter = [this, is_hoverable = args.is_hoverable]() {
		if (is_hoverable())
		{
			bkg().fill_color = hover_fill;
			bkg().update_fill_color().send_buffer();
			Machine.main_window->request_cursor(&wh, StandardCursor::HAND);
		}
		else
			hovering = false;
		};
	on_hover_exit = [this, is_hoverable = args.is_hoverable]() {
		bkg().fill_color = *prev_fill;
		bkg().update_fill_color().send_buffer();
		Machine.main_window->release_cursor(&wh);
		};
}

ToggleButton::ToggleButton(const StandardButtonArgs& args)
	: StandardButton(args)
{
	on_release = [this](const MouseButtonEvent& mb, Position pos) {
		if (!selected)
		{
			if (is_selectable(mb, pos))
				select(mb, pos);
		}
		else
		{
			if (is_deselectable(mb, pos))
				deselect(mb, pos);
		}
		};
}

void ToggleButton::select(const MouseButtonEvent& mb, Position local_pos)
{
	if (selected) return;
	bkg().fill_color = select_fill;
	bkg().update_fill_color().send_buffer();
	prev_fill = &select_fill;
	selected = true;
	Machine.main_window->release_cursor(&wh);
	on_select(mb, local_pos);
}

void ToggleButton::deselect(const MouseButtonEvent& mb, Position local_pos)
{
	if (!selected) return;
	bkg().fill_color = normal_fill;
	bkg().update_fill_color().send_buffer();
	prev_fill = &normal_fill;
	selected = false;
	Machine.main_window->release_cursor(&wh);
	on_deselect(mb, local_pos);
}
