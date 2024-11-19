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

	void init(const WidgetPlacement& wp, std::shared_ptr<TextRender>&& txt);

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
	
	virtual void draw() override;
	void send_vp() const;
	void process();
	bool is_pressed(MouseButton mb) const;
	bool is_hovered(Position* local_pos = nullptr) const;
	void unhover();

	TextRender& text() { return tr_wget(*this, TEXT); }
	const TextRender& text() const { return tr_wget(*this, TEXT); }
};

inline std::function<void(const MouseButtonEvent&, Position)> fconv_on_action(std::function<void()>&& f)
{ return [f = std::move(f)](const MouseButtonEvent&, Position) { f(); }; }
inline std::function<void(const MouseButtonEvent&, Position)> fconv_on_action(std::function<void(const MouseButtonEvent&)>&& f)
{ return [f = std::move(f)](const MouseButtonEvent& m, Position) { f(m); }; }

inline TButton& b_t_wget(Widget& w, size_t i)
{
	return *w.get<TButton>(i);
}

inline const TButton& b_t_wget(const Widget& w, size_t i)
{
	return *w.get<TButton>(i);
}

enum class ButtonGState : char
{
	NORMAL,
	HOVERED,
	PRESSED,
	DISABLED,
	_NONE
};

struct TButtonGDesign
{
	RGBA border;
	RGBA fill;
	RGBA text_color = RGBA::WHITE;
};

struct StandardTButtonArgs
{
	MouseButtonHandler* mb_parent;
	FontRange* frange = Fonts::label_regular;
	Shader* rr_shader;
	glm::mat3* vp;
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
	std::function<bool(const MouseButtonEvent&, Position)> is_selectable = [](const MouseButtonEvent& mb, Position) { return mb.button == MouseButton::LEFT; };
	std::function<void(const MouseButtonEvent&, Position)> on_select = [](const MouseButtonEvent&, Position) {};

	StandardTButtonArgs(MouseButtonHandler* mb_parent, Shader* rr_shader, glm::mat3* vp) : mb_parent(mb_parent), rr_shader(rr_shader), vp(vp) {}
};

inline std::function<bool(const MouseButtonEvent&, Position)> fconv_st_check(std::function<bool()>&& f)
{ return [f = std::move(f)](const MouseButtonEvent&, Position) { return f(); }; }
inline std::function<bool(const MouseButtonEvent&, Position)> fconv_st_check(std::function<bool(const MouseButtonEvent&)>&& f)
{ return [f = std::move(f)](const MouseButtonEvent& m, Position) { return f(m); }; }
inline std::function<void(const MouseButtonEvent&, Position)> fconv_st_on_action(std::function<void()>&& f)
{ return [f = std::move(f)](const MouseButtonEvent&, Position) { f(); }; }
inline std::function<void(const MouseButtonEvent&, Position)> fconv_st_on_action(std::function<void(const MouseButtonEvent&)>&& f)
{ return [f = std::move(f)](const MouseButtonEvent& m, Position) { f(m); }; }

struct StandardTButton : public TButton
{
	TButtonGDesign g_normal, g_hovered, g_pressed, g_disabled;
	ButtonGState state = ButtonGState::_NONE;

	std::function<bool()> is_hoverable;
	std::function<bool(const MouseButtonEvent&, Position)> is_selectable;
	std::function<void(const MouseButtonEvent&, Position)> on_select;
	
	StandardTButton(const StandardTButtonArgs& args);

	void select(const MouseButtonEvent& mb = MouseButtonEvent::LEFT_CLICK, Position local_pos = {});

	void send_state(ButtonGState _state);
	// LATER implement disabled gstate
};

inline StandardTButton& sb_t_wget(Widget& w, size_t i)
{
	return *w.get<StandardTButton>(i);
}

inline const StandardTButton& sb_t_wget(const Widget& w, size_t i)
{
	return *w.get<StandardTButton>(i);
}

struct ToggleTButtonArgs
{
	MouseButtonHandler* mb_parent;
	FontRange* frange = Fonts::label_regular;
	Shader* rr_shader;
	glm::mat3* vp;
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
	std::function<bool(const MouseButtonEvent&, Position)> is_selectable = [](const MouseButtonEvent& mb, Position) { return mb.button == MouseButton::LEFT; };
	std::function<void(const MouseButtonEvent&, Position)> on_select = [](const MouseButtonEvent&, Position) {};
	std::function<bool(const MouseButtonEvent&, Position)> is_deselectable = [](const MouseButtonEvent& mb, Position) { return mb.button == MouseButton::LEFT; };
	std::function<void(const MouseButtonEvent&, Position)> on_deselect = [](const MouseButtonEvent&, Position) {};

	ToggleTButtonArgs(MouseButtonHandler* mb_parent, Shader* rr_shader, glm::mat3* vp) : mb_parent(mb_parent), rr_shader(rr_shader), vp(vp) {}
};

inline std::function<bool(const MouseButtonEvent&, Position)> fconv_tt_check(std::function<bool()>&& f)
{ return [f = std::move(f)](const MouseButtonEvent&, Position) { return f(); }; }
inline std::function<bool(const MouseButtonEvent&, Position)> fconv_tt_check(std::function<bool(const MouseButtonEvent&)>&& f)
{ return [f = std::move(f)](const MouseButtonEvent& m, Position) { return f(m); }; }
inline std::function<void(const MouseButtonEvent&, Position)> fconv_tt_on_action(std::function<void()>&& f)
{ return [f = std::move(f)](const MouseButtonEvent&, Position) { f(); }; }
inline std::function<void(const MouseButtonEvent&, Position)> fconv_tt_on_action(std::function<void(const MouseButtonEvent&)>&& f)
{ return [f = std::move(f)](const MouseButtonEvent& m, Position) { f(m); }; }

struct ToggleTButton : public TButton
{
	TButtonGDesign g_normal, g_hovered, g_pressed, g_disabled;
	ButtonGState state = ButtonGState::_NONE;

private:
	bool selected = false;

public:
	std::function<bool()> is_hoverable;
	std::function<bool(const MouseButtonEvent&, Position)> is_selectable;
	std::function<void(const MouseButtonEvent&, Position)> on_select;
	std::function<bool(const MouseButtonEvent&, Position)> is_deselectable;
	std::function<void(const MouseButtonEvent&, Position)> on_deselect;

	ToggleTButton(const ToggleTButtonArgs& args);

	void select(const MouseButtonEvent& mb = MouseButtonEvent::LEFT_CLICK, Position local_pos = {});
	void deselect(const MouseButtonEvent& mb = MouseButtonEvent::LEFT_CLICK, Position local_pos = {});
	void unclick();

	void send_state(ButtonGState _state);
	// LATER implement disabled gstate
};

inline ToggleTButton& tb_t_wget(Widget& w, size_t i)
{
	return *w.get<ToggleTButton>(i);
}

inline const ToggleTButton& tb_t_wget(const Widget& w, size_t i)
{
	return *w.get<ToggleTButton>(i);
}

class ToggleTButtonGroup
{
	std::unordered_map<size_t, ToggleTButton*> buttons;
	size_t current_btn = -1;

public:
	ToggleTButtonGroup() = default;
	ToggleTButtonGroup(const ToggleTButtonGroup&) = delete;
	ToggleTButtonGroup(ToggleTButtonGroup&&) noexcept = delete;
	
	void init(std::unordered_map<size_t, ToggleTButton*>&& buttons, size_t starting_btn);
	void select(size_t btn);
	void draw();
};

// LATER IButton, StandardIButton, ToggleIButton for image buttons. Use b_i_wget, sb_i_wget, and tb_i_wget.
