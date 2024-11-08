#pragma once

#include "../text/TextRender.h"
#include "RoundRect.h"
#include "user/Platform.h"
#include "../text/CommonFonts.h"

struct Button : public Widget
{
	enum
	{
		TEXT,
		BKG,
		_W_COUNT
	};
	
protected:
	Shader* bkg_shader;
	MouseButtonHandler* parent_mbh;
	MouseButtonHandler handler;
	bool left_pressed = false;
	bool middle_pressed = false;
	bool right_pressed = false;

	bool hovering = false;

public:
	bool enabled = true;
protected:

	WindowHandle wh;

	void init(const WidgetPlacement& wp, TextRender* text, RoundRect* bkg);

public:
	glm::mat3* vp;
	std::function<void(const MouseButtonEvent&, Position)> on_press = [](const MouseButtonEvent&, Position) {};
	std::function<void(const MouseButtonEvent&, Position)> on_release = [](const MouseButtonEvent&, Position) {};
	std::function<void()> on_hover_enter = []() {};
	std::function<void()> on_hover_exit = []() {}; // LATER define simple functor utility

	Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler* parent_mbh, const UTF::String& text);
	Button(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler* parent_mbh, UTF::String&& text);
	Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler* parent_mbh, const UTF::String& text);
	Button(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler* parent_mbh, UTF::String&& text);
	Button(const Button&) = delete;
	Button(Button&&) noexcept = delete;
	~Button();

	void draw() const;
	void send_vp() const;
	void process();
	bool is_pressed(MouseButton mb) const;
	bool is_hovered(Position* local_pos = nullptr) const;

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

struct StandardButtonArgs
{
	MouseButtonHandler* mb_parent = nullptr;
	FontRange& frange = *Fonts::label_regular;
	Shader* rr_shader = nullptr;
	glm::mat3* vp = nullptr;
	float font_size = 18.0f;
	UTF::String text = "";
	FlatTransform transform = {};
	Scale bkg_size = {};
	float thickness = 0.5f;
	float corner_radius = 5.0f;
	RGBA border_color = RGBA(HSV(0.7f, 0.5f, 0.2f).to_rgb(), 1.0f);
	RGBA normal_fill = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.9f);
	std::function<bool()> is_hoverable = []() { return true; };
};

struct StandardButton : public Button
{
	RGBA normal_fill;
	RGBA hover_fill = RGBA(HSV(0.7f, 0.1f, 0.5f).to_rgb(), 0.9f);

protected:
	RGBA* prev_fill = nullptr;
public:

	StandardButton(const StandardButtonArgs& args);
};

inline StandardButton& sb_wget(Widget& w, size_t i)
{
	return *w.get<StandardButton>(i);
}

inline const StandardButton& sb_wget(const Widget& w, size_t i)
{
	return *w.get<StandardButton>(i);
}

struct ToggleButton : public StandardButton
{
	RGBA select_fill = RGBA(HSV(0.7f, 0.3f, 0.5f).to_rgb(), 0.9f);
	std::function<bool(const MouseButtonEvent&, Position)> is_selectable = [](const MouseButtonEvent& mb, Position) { return mb.button == MouseButton::LEFT; };
	std::function<void(const MouseButtonEvent&, Position)> on_select = [](const MouseButtonEvent&, Position) {};
	std::function<bool(const MouseButtonEvent&, Position)> is_deselectable = [](const MouseButtonEvent& mb, Position) { return mb.button == MouseButton::LEFT; };
	std::function<void(const MouseButtonEvent&, Position)> on_deselect = [](const MouseButtonEvent&, Position) {};

private:
	bool selected = false;
public:

	ToggleButton(const StandardButtonArgs& args);

	void select(const MouseButtonEvent& mb = MouseButtonEvent::LEFT_CLICK, Position local_pos = {});
	void deselect(const MouseButtonEvent& mb = MouseButtonEvent::LEFT_CLICK, Position local_pos = {});
};

inline ToggleButton& tb_wget(Widget& w, size_t i)
{
	return *w.get<ToggleButton>(i);
}

inline const ToggleButton& tb_wget(const Widget& w, size_t i)
{
	return *w.get<ToggleButton>(i);
}
