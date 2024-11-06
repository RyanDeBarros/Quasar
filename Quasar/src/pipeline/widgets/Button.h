#pragma once

#include "../text/TextRender.h"
#include "RoundRect.h"
#include "user/Platform.h"

struct Button : public Widget
{
	enum
	{
		TEXT,
		BKG,
		_W_COUNT
	};
	
private:
	Shader bkg_shader;
	MouseButtonHandler& parent;
	MouseButtonHandler handler;

	void init(const WidgetPlacement& wp, TextRender* text, RoundRect* bkg);

public:
	std::function<void(const MouseButtonEvent&)> on_press = [](const MouseButtonEvent&) {};
	std::function<void(const MouseButtonEvent&)> on_release = [](const MouseButtonEvent&) {};
	std::function<void()> on_hover = []() {};

	Button(const WidgetPlacement& wp, Font* font, Shader&& bkg_shader, MouseButtonHandler& parent, const UTF::String& text);
	Button(const WidgetPlacement& wp, Font* font, Shader&& bkg_shader, MouseButtonHandler& parent, UTF::String&& text);
	Button(const WidgetPlacement& wp, FontRange& frange, float font_size, Shader&& bkg_shader, MouseButtonHandler& parent, const UTF::String& text);
	Button(const WidgetPlacement& wp, FontRange& frange, float font_size, Shader&& bkg_shader, MouseButtonHandler& parent, UTF::String&& text);
	Button(const Button&) = delete;
	Button(Button&&) noexcept = delete;
	~Button();

	void draw() const;
	void process();
	void send_vp(const glm::mat3& vp, FlatTransform parent);

	RoundRect& bkg() { return rr_wget(*this, BKG); }
	const RoundRect& bkg() const { return rr_wget(*this, BKG); }
	TextRender& text() { return tr_wget(*this, TEXT); }
	const TextRender& text() const { return tr_wget(*this, TEXT); }
};

inline Button& b_wget(Widget& w, size_t i)
{
	return *w.get<Button>(i);
}

inline const Button& b_wget(const Widget& w, size_t i)
{
	return *w.get<Button>(i);
}
