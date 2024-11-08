#pragma once

#include "../text/TextRender.h"
#include "RoundRect.h"
#include "user/Platform.h"
#include "../text/CommonFonts.h"

struct TButton : public RoundRect
{
	enum
	{
		TEXT,
		_W_COUNT
	};
	
protected:
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

	void init(const WidgetPlacement& wp, TextRender* text);

public:
	glm::mat3* vp;
	std::function<void(const MouseButtonEvent&, Position)> on_press = [](const MouseButtonEvent&, Position) {};
	std::function<void(const MouseButtonEvent&, Position)> on_release = [](const MouseButtonEvent&, Position) {};
	std::function<void()> on_hover_enter = []() {};
	std::function<void()> on_hover_exit = []() {}; // LATER define simple functor utility

	TButton(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler* parent_mbh, const UTF::String& text);
	TButton(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler* parent_mbh, UTF::String&& text);
	TButton(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler* parent_mbh, const UTF::String& text);
	TButton(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler* parent_mbh, UTF::String&& text);
	TButton(const TButton&) = delete;
	TButton(TButton&&) noexcept = delete;
	~TButton();

	virtual void draw() override;
	void send_vp() const;
	void process();
	bool is_pressed(MouseButton mb) const;
	bool is_hovered(Position* local_pos = nullptr) const;

	TextRender& text() { return tr_wget(*this, TEXT); }
	const TextRender& text() const { return tr_wget(*this, TEXT); }
};

inline TButton& b_wget(Widget& w, size_t i)
{
	return *w.get<TButton>(i);
}

inline const TButton& b_wget(const Widget& w, size_t i)
{
	return *w.get<TButton>(i);
}

enum class ButtonGState
{
	NORMAL,
	HOVERED,
	PRESSED,
	DISABLED
};

struct TButtonGDesign
{
	RGBA border;
	RGBA fill;
	RGBA text_color = RGBA::WHITE;
};

struct StandardTButton;

struct StandardTButtonArgs
{
	MouseButtonHandler* mb_parent = nullptr;
	FontRange& frange = *Fonts::label_regular;
	Shader* rr_shader = nullptr;
	glm::mat3* vp = nullptr;
	float font_size = 18.0f;
	UTF::String text = "";
	glm::vec2 pivot = { 0.5f, 0.5f };
	FlatTransform transform = {};
	float thickness = 0.5f;
	float corner_radius = 5.0f;
	TButtonGDesign normal = { RGBA(HSV(0.7f, 0.5f, 0.2f).to_rgb(), 1.0f), RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.9f) };
	TButtonGDesign hovered = { RGBA(HSV(0.7f, 0.5f, 0.2f).to_rgb(), 1.0f), RGBA(HSV(0.7f, 0.1f, 0.5f).to_rgb(), 0.9f) };
	TButtonGDesign pressed = { RGBA(HSV(0.7f, 0.5f, 0.2f).to_rgb(), 1.0f), RGBA(HSV(0.7f, 0.3f, 0.5f).to_rgb(), 0.9f) };
	TButtonGDesign disabled = { RGBA(HSV(0.7f, 0.5f, 0.2f).to_rgb(), 1.0f), RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.9f) };

	std::function<bool()> is_hoverable = []() { return true; };
	std::function<bool(StandardTButton&, const MouseButtonEvent&, Position)> is_selectable = [](StandardTButton&, const MouseButtonEvent& mb, Position) { return mb.button == MouseButton::LEFT; };
	std::function<void(StandardTButton&, const MouseButtonEvent&, Position)> on_select = [](StandardTButton&, const MouseButtonEvent&, Position) {};
};

struct StandardTButton : public TButton
{
	ButtonGState state = ButtonGState::NORMAL;
	TButtonGDesign g_normal, g_hovered, g_pressed, g_disabled;

	std::function<bool()> is_hoverable;
	std::function<bool(StandardTButton&, const MouseButtonEvent&, Position)> is_selectable;
	std::function<void(StandardTButton&, const MouseButtonEvent&, Position)> on_select;
	
	StandardTButton(const StandardTButtonArgs& args);

	void send_state(ButtonGState _state);
	// LATER implement disabled gstate
};

inline StandardTButton& sb_wget(Widget& w, size_t i)
{
	return *w.get<StandardTButton>(i);
}

inline const StandardTButton& sb_wget(const Widget& w, size_t i)
{
	return *w.get<StandardTButton>(i);
}

struct ToggleTButtonArgs
{
	MouseButtonHandler* mb_parent = nullptr;
	FontRange& frange = *Fonts::label_regular;
	Shader* rr_shader = nullptr;
	glm::mat3* vp = nullptr;
	float font_size = 18.0f;
	UTF::String text = "";
	glm::vec2 pivot = { 0.5f, 0.5f };
	FlatTransform transform = {};
	float thickness = 0.5f;
	float corner_radius = 5.0f;
	RGBA border_color = RGBA(HSV(0.7f, 0.5f, 0.2f).to_rgb(), 1.0f);
	RGBA normal_fill = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.9f);
	std::function<bool()> is_hoverable = []() { return true; };
};

struct ToggleTButton : public TButton
{
	RGBA normal_fill;
	RGBA hover_fill = RGBA(HSV(0.7f, 0.1f, 0.5f).to_rgb(), 0.9f);
	RGBA select_fill = RGBA(HSV(0.7f, 0.3f, 0.5f).to_rgb(), 0.9f);

protected:
	RGBA* prev_fill = nullptr;
public:

	std::function<bool(const MouseButtonEvent&, Position)> is_selectable = [](const MouseButtonEvent& mb, Position) { return mb.button == MouseButton::LEFT; };
	std::function<void(const MouseButtonEvent&, Position)> on_select = [](const MouseButtonEvent&, Position) {};
	std::function<bool(const MouseButtonEvent&, Position)> is_deselectable = [](const MouseButtonEvent& mb, Position) { return mb.button == MouseButton::LEFT; };
	std::function<void(const MouseButtonEvent&, Position)> on_deselect = [](const MouseButtonEvent&, Position) {};

private:
	bool selected = false;
public:

	ToggleTButton(const ToggleTButtonArgs& args);

	void select(const MouseButtonEvent& mb = MouseButtonEvent::LEFT_CLICK, Position local_pos = {});
	void deselect(const MouseButtonEvent& mb = MouseButtonEvent::LEFT_CLICK, Position local_pos = {});
};

inline ToggleTButton& tb_wget(Widget& w, size_t i)
{
	return *w.get<ToggleTButton>(i);
}

inline const ToggleTButton& tb_wget(const Widget& w, size_t i)
{
	return *w.get<ToggleTButton>(i);
}

// LATER IButton, StandardIButton, ToggleIButton for image buttons.
