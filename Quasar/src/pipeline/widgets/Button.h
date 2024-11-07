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
	Shader* bkg_shader;
	MouseButtonHandler& parent_mbh;
	MouseButtonHandler handler;
	bool left_pressed = false;
	bool middle_pressed = false;
	bool right_pressed = false;

	void init(const WidgetPlacement& wp, TextRender* text, RoundRect* bkg);

public:
	glm::mat3* vp;
	std::function<void(const MouseButtonEvent&, Position)> on_press = [](const MouseButtonEvent&, Position) {};
	std::function<void(const MouseButtonEvent&, Position)> on_release = [](const MouseButtonEvent&, Position) {};
	std::function<void()> on_hover_enter = []() {};
	std::function<void()> on_hover_exit = []() {}; // LATER define simple functor utility

	Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler& parent_mbh, const UTF::String& text);
	Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler& parent_mbh, UTF::String&& text);
	Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler& parent_mbh, const UTF::String& text);
	Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler& parent_mbh, UTF::String&& text);
	Button(const Button&) = delete;
	Button(Button&&) noexcept = delete;
	~Button();

	void draw() const;
	void send_vp() const;
	void process();
	bool is_pressed(MouseButton mb) const;
	bool is_hovered(Position* local_pos = nullptr) const;

	bool hovering = false;

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
