#include "Button.h"

void Button::init(const WidgetPlacement& wp, TextRender* text, RoundRect* bkg)
{
	children.resize(_W_COUNT);
	assign_widget(this, TEXT, text);
	assign_widget(this, BKG, bkg);
	parent.children.push_back(&handler);
	handler.callback = [this](const MouseButtonEvent& m)
		{
			if (m.action == IAction::PRESS)
				on_press(m);
			else if (m.action == IAction::RELEASE)
				on_release(m);
		};
	self = wp;
}

Button::Button(const WidgetPlacement& wp, Font* font, Shader&& bkg_shader, MouseButtonHandler& parent, const UTF::String& text)
	: bkg_shader(std::move(bkg_shader)), parent(parent)
{
	init(wp, new TextRender(font, text), new RoundRect(&this->bkg_shader));
}

Button::Button(const WidgetPlacement& wp, Font* font, Shader&& bkg_shader, MouseButtonHandler& parent, UTF::String&& text)
	: bkg_shader(std::move(bkg_shader)), parent(parent)
{
	init(wp, new TextRender(font, std::move(text)), new RoundRect(&this->bkg_shader));
}

Button::Button(const WidgetPlacement& wp, FontRange& frange, float font_size, Shader&& bkg_shader, MouseButtonHandler& parent, const UTF::String& text)
	: bkg_shader(std::move(bkg_shader)), parent(parent)
{
	init(wp, new TextRender(frange, font_size, text), new RoundRect(&this->bkg_shader));
}

Button::Button(const WidgetPlacement& wp, FontRange& frange, float font_size, Shader&& bkg_shader, MouseButtonHandler& parent, UTF::String&& text)
	: bkg_shader(std::move(bkg_shader)), parent(parent)
{
	init(wp, new TextRender(frange, font_size, std::move(text)), new RoundRect(&this->bkg_shader));
}

Button::~Button()
{
	parent.remove_child(&handler);
}

void Button::draw() const
{
	bkg().ur->draw();
	text().draw();
}

void Button::process()
{
	// TODO
	
}

void Button::send_vp(const glm::mat3& vp, FlatTransform parent)
{
	text().send_vp(vp * global_matrix());
	bkg().update_transform().send_buffer();
}
