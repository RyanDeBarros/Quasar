#include "Button.h"

#include "user/Machine.h"

void Button::init(const WidgetPlacement& wp, TextRender* txt, RoundRect* bkg)
{
	children.resize(_W_COUNT);
	assign_widget(this, TEXT, txt);
	assign_widget(this, BKG, bkg);
	parent_mbh.children.push_back(&handler);
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

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler& parent_mbh, const UTF::String& txt)
	: bkg_shader(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(font, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(txt);
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler& parent_mbh, UTF::String&& txt)
	: bkg_shader(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(font, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(std::move(txt));
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler& parent_mbh, const UTF::String& txt)
	: bkg_shader(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(frange, font_size, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(txt);
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler& parent_mbh, UTF::String&& txt)
	: bkg_shader(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(frange, font_size, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(std::move(txt));
}

Button::~Button()
{
	parent_mbh.remove_child(&handler);
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
