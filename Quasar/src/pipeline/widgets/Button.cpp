#include "Button.h"

#include "user/Machine.h"

void Button::init(const WidgetPlacement& wp, TextRender* txt, RoundRect* bkg)
{
	children.resize(_W_COUNT);
	assign_widget(this, TEXT, txt);
	assign_widget(this, BKG, bkg);
	parent.children.push_back(&handler);
	handler.callback = [this](const MouseButtonEvent& m) {
		Position local_cursor_pos;
		if (contains_cursor(local_cursor_pos))
		{
			if (m.action == IAction::PRESS)
			{
				on_press(m, local_cursor_pos);
				m.consumed = true;
				pressed = true;
			}
			else if (m.action == IAction::RELEASE)
			{
				on_release(m, local_cursor_pos);
				m.consumed = true;
				pressed = false;
			}
		}
		};
	self = wp;
	text().format.horizontal_align = TextRender::HorizontalAlign::CENTER;
	text().format.vertical_align = TextRender::VerticalAlign::MIDDLE;
}

bool Button::contains_cursor(Position& pos) const
{
	pos = local_of(Machine.cursor_world_coordinates(glm::inverse(*vp)));
	return bkg().self.contains_point(pos);
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler& parent, const UTF::String& txt)
	: bkg_shader(bkg_shader), parent(parent), vp(vp)
{
	init(wp, new TextRender(font, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(txt);
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler& parent, UTF::String&& txt)
	: bkg_shader(bkg_shader), parent(parent), vp(vp)
{
	init(wp, new TextRender(font, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(std::move(txt));
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler& parent, const UTF::String& txt)
	: bkg_shader(bkg_shader), parent(parent), vp(vp)
{
	init(wp, new TextRender(frange, font_size, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(txt);
}

Button::Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler& parent, UTF::String&& txt)
	: bkg_shader(bkg_shader), parent(parent), vp(vp)
{
	init(wp, new TextRender(frange, font_size, "", { 0.5f, 0.5f }), new RoundRect(this->bkg_shader));
	text().set_text(std::move(txt));
}

Button::~Button()
{
	parent.remove_child(&handler);
}

void Button::draw() const
{
	bkg().draw();
	text().draw();
}

void Button::process() const
{
	Position local_cursor_pos;
	if (contains_cursor(local_cursor_pos))
		on_hover(local_cursor_pos);
}

void Button::send_vp() const
{
	bkg().update_transform().send_buffer();
	text().send_vp(*vp);
}
