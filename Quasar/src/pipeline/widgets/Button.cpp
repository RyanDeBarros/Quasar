#include "Button.h"

#include "user/Machine.h"

void TButton::init(const WidgetPlacement& wp, TextRender* txt)
{
	children.resize(_W_COUNT);
	assign_widget(this, TEXT, txt);
	if (parent_mbh)
		parent_mbh->children.push_back(&handler);
	handler.callback = [this](const MouseButtonEvent& m) {
		if (m.action == IAction::PRESS)
		{
			Position local_cursor_pos;
			if (is_hovered(&local_cursor_pos))
			{
				if (enabled)
				{
					m.consumed = true;
					on_press(m, local_cursor_pos);
				}
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
			if (enabled && is_hovered(&local_cursor_pos))
			{
				if (is_pressed(m.button))
				{
					m.consumed = true;
					on_release(m, local_cursor_pos);
				}
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
	text().self.transform.scale = text().self.transform.scale / self.transform.scale;
}

TButton::TButton(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler* parent_mbh, const UTF::String& txt)
	: RoundRect(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(font, "", { 0.5f, 0.5f }));
	text().set_text(txt);
}

TButton::TButton(glm::mat3* vp, const WidgetPlacement& wp, Font* font, Shader* bkg_shader, MouseButtonHandler* parent_mbh, UTF::String&& txt)
	: RoundRect(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(font, "", { 0.5f, 0.5f }));
	text().set_text(std::move(txt));
}

TButton::TButton(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler* parent_mbh, const UTF::String& txt)
	: RoundRect(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(frange, font_size, "", { 0.5f, 0.5f }));
	text().set_text(txt);
}

TButton::TButton(glm::mat3* vp, const WidgetPlacement& wp, FontRange& frange, float font_size, Shader* bkg_shader, MouseButtonHandler* parent_mbh, UTF::String&& txt)
	: RoundRect(bkg_shader), parent_mbh(parent_mbh), vp(vp)
{
	init(wp, new TextRender(frange, font_size, "", { 0.5f, 0.5f }));
	text().set_text(std::move(txt));
}

TButton::~TButton()
{
	if (parent_mbh)
		parent_mbh->remove_child(&handler);
}

void TButton::draw()
{
	RoundRect::draw();
	text().draw();
}

void TButton::send_vp() const
{
	update_transform().send_buffer();
	text().send_vp(*vp);
}

void TButton::process()
{
	if (!enabled) return;
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

bool TButton::is_pressed(MouseButton mb) const
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

bool TButton::is_hovered(Position* local_pos) const
{
	Position pos = local_of(Machine.cursor_world_pos(glm::inverse(*vp)));
	if (local_pos)
		*local_pos = pos;
	return on_interval(pos.x, -0.5f, 0.5f) && on_interval(pos.y, -0.5f, 0.5f);
}

StandardTButton::StandardTButton(const StandardTButtonArgs& args)
	: TButton(args.vp, { args.transform, args.pivot }, args.frange, args.font_size, args.rr_shader, args.mb_parent, args.text),
	g_normal(args.normal), g_hovered(args.hovered), g_pressed(args.pressed), g_disabled(args.disabled),
	is_hoverable(args.is_hoverable), is_selectable(args.is_selectable), on_select(args.on_select)
{
	thickness = args.thickness;
	corner_radius = args.corner_radius;
	update_all();
	send_state(ButtonGState::NORMAL);
	on_hover_enter = [this]() {
		if (is_hoverable())
		{
			send_state(ButtonGState::HOVERED);
			Machine.main_window->request_cursor(&wh, StandardCursor::HAND);
			if (!(wh.flags & WindowHandle::OWN_CURSOR))
				hovering = false;
		}
		else
			hovering = false;
		};
	on_hover_exit = [this]() {
		send_state(ButtonGState::NORMAL);
		Machine.main_window->release_cursor(&wh);
		};
	on_press = [this](const MouseButtonEvent& mb, Position pos) {
		send_state(ButtonGState::PRESSED);
		Machine.main_window->release_cursor(&wh);
		};
	on_release = [this](const MouseButtonEvent& mb, Position pos) {
		if (is_selectable(*this, mb, pos))
			select(mb, pos);
		};
}

void StandardTButton::select(const MouseButtonEvent& mb, Position local_pos)
{
	send_state(ButtonGState::NORMAL);
	hovering = false;
	on_select(*this, mb, local_pos);
}

void StandardTButton::send_state(ButtonGState _state)
{
	if (state == _state)
		return;
	state = _state;
	if (state == ButtonGState::NORMAL)
	{
		border_color = g_normal.border;
		fill_color = g_normal.fill;
		update_fill_color().update_border_color().send_buffer();
		text().fore_color = g_normal.text_color;
		text().send_fore_color();
	}
	else if (state == ButtonGState::HOVERED)
	{
		border_color = g_hovered.border;
		fill_color = g_hovered.fill;
		update_fill_color().update_border_color().send_buffer();
		text().fore_color = g_hovered.text_color;
		text().send_fore_color();
	}
	else if (state == ButtonGState::PRESSED)
	{
		border_color = g_pressed.border;
		fill_color = g_pressed.fill;
		update_fill_color().update_border_color().send_buffer();
		text().fore_color = g_pressed.text_color;
		text().send_fore_color();
	}
	else if (state == ButtonGState::DISABLED)
	{
		border_color = g_disabled.border;
		fill_color = g_disabled.fill;
		update_fill_color().update_border_color().send_buffer();
		text().fore_color = g_disabled.text_color;
		text().send_fore_color();
	}
}

ToggleTButton::ToggleTButton(const ToggleTButtonArgs& args)
	: TButton(args.vp, { args.transform, args.pivot }, args.frange, args.font_size, args.rr_shader, args.mb_parent, args.text),
	g_normal(args.normal), g_hovered(args.hovered), g_pressed(args.pressed), g_disabled(args.disabled),
	is_hoverable(args.is_hoverable), is_selectable(args.is_selectable), on_select(args.on_select), is_deselectable(args.is_deselectable), on_deselect(args.on_deselect)
{
	thickness = args.thickness;
	corner_radius = args.corner_radius;
	update_all();
	send_state(ButtonGState::NORMAL);
	on_hover_enter = [this]() {
		if (is_hoverable())
		{
			send_state(ButtonGState::HOVERED);
			Machine.main_window->request_cursor(&wh, StandardCursor::HAND);
			if (!(wh.flags & WindowHandle::OWN_CURSOR))
				hovering = false;
		}
		else
			hovering = false;
		};
	on_hover_exit = [this]() {
		if (selected)
			send_state(ButtonGState::PRESSED);
		else
			send_state(ButtonGState::NORMAL);
		Machine.main_window->release_cursor(&wh);
		};
	on_press = [this](const MouseButtonEvent& mb, Position pos) {
		send_state(ButtonGState::PRESSED);
		Machine.main_window->release_cursor(&wh);
		};
	on_release = [this](const MouseButtonEvent& mb, Position pos) {
		if (selected)
		{
			if (is_deselectable(*this, mb, pos))
				deselect(mb, pos);
			else
				unclick();
		}
		else
		{
			if (is_selectable(*this, mb, pos))
				select(mb, pos);
			else
				unclick();
		}
		};
}

void ToggleTButton::select(const MouseButtonEvent& mb, Position local_pos)
{
	if (selected)
		return;
	selected = true;
	send_state(ButtonGState::PRESSED);
	hovering = false;
	on_select(*this, mb, local_pos);
}

void ToggleTButton::deselect(const MouseButtonEvent& mb, Position local_pos)
{
	if (!selected)
		return;
	selected = false;
	send_state(ButtonGState::NORMAL);
	hovering = false;
	on_deselect(*this, mb, local_pos);
}

void ToggleTButton::unclick()
{
	if (selected)
		send_state(ButtonGState::PRESSED);
	else
		send_state(ButtonGState::NORMAL);
	hovering = false;
}

void ToggleTButton::send_state(ButtonGState _state)
{
	if (state == _state)
		return;
	state = _state;
	if (state == ButtonGState::NORMAL)
	{
		border_color = g_normal.border;
		fill_color = g_normal.fill;
		update_fill_color().update_border_color().send_buffer();
		text().fore_color = g_normal.text_color;
		text().send_fore_color();
	}
	else if (state == ButtonGState::HOVERED)
	{
		border_color = g_hovered.border;
		fill_color = g_hovered.fill;
		update_fill_color().update_border_color().send_buffer();
		text().fore_color = g_hovered.text_color;
		text().send_fore_color();
	}
	else if (state == ButtonGState::PRESSED)
	{
		border_color = g_pressed.border;
		fill_color = g_pressed.fill;
		update_fill_color().update_border_color().send_buffer();
		text().fore_color = g_pressed.text_color;
		text().send_fore_color();
	}
	else if (state == ButtonGState::DISABLED)
	{
		border_color = g_disabled.border;
		fill_color = g_disabled.fill;
		update_fill_color().update_border_color().send_buffer();
		text().fore_color = g_disabled.text_color;
		text().send_fore_color();
	}
}

void ToggleTButtonGroup::init(std::unordered_map<size_t, ToggleTButton*>&& _buttons, size_t starting_btn)
{
	buttons = std::move(_buttons);
	for (auto iter = buttons.begin(); iter != buttons.end(); ++iter)
		iter->second->is_deselectable = [](ToggleTButton&, const MouseButtonEvent&, Position) { return false; };
	current_btn = starting_btn;
	buttons.find(current_btn)->second->select();
}

void ToggleTButtonGroup::select(size_t btn)
{
	if (btn != current_btn)
	{
		auto iter = buttons.find(btn);
		if (iter != buttons.end())
		{
			buttons.find(current_btn)->second->deselect();
			iter->second->select();
			current_btn = btn;
		}
	}
}

void ToggleTButtonGroup::draw()
{
	for (auto iter = buttons.begin(); iter != buttons.end(); ++iter)
		iter->second->draw();
}
