#include "ColorPicker.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"
#include "edit/Color.h"
#include "user/Machine.h"
#include "user/GUI.h"
#include "../render/Uniforms.h"
#include "RoundRect.h"
#include "../text/TextRender.h"
#include "../text/CommonFonts.h"
#include "Button.h"

void ColorPicker::send_gradient_color_uniform(const Shader& shader, GradientIndex index, ColorFrame color)
{
	Uniforms::send_4(shader, "u_GradientColors[0]", color.rgba().as_vec(), (GLint)index);
}

// LATER maybe input handler connections should be made by parent, not child, so that there's no need to pass them in constructor.
ColorPicker::ColorPicker(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler)
	: quad_shader(FileSystem::shader_path("gradients/quad.vert"), FileSystem::shader_path("gradients/quad.frag.tmpl"),
		{ {"$MAX_GRADIENT_COLORS", std::to_string((int)GradientIndex::_MAX_GRADIENT_COLORS) } }),
	linear_hue_shader(FileSystem::shader_path("gradients/linear_hue.vert"), FileSystem::shader_path("gradients/linear_hue.frag")),
	hue_wheel_w_shader(FileSystem::shader_path("gradients/hue_wheel_w.vert"), FileSystem::shader_path("gradients/hue_wheel_w.frag")),
	linear_lightness_shader(FileSystem::shader_path("gradients/linear_lightness.vert"), FileSystem::shader_path("gradients/linear_lightness.frag")),
	circle_cursor_shader(FileSystem::shader_path("circle_cursor.vert"), FileSystem::shader_path("circle_cursor.frag")),
	round_rect_shader(FileSystem::shader_path("round_rect.vert"), FileSystem::shader_path("round_rect.frag")),
	Widget(_W_COUNT), parent_mb_handler(parent_mb_handler), parent_key_handler(parent_key_handler), vp(vp)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::BLACK, ColorFrame(HSV(0.0f, 0.0f, 0.0f)));
	send_gradient_color_uniform(quad_shader, GradientIndex::WHITE, ColorFrame(HSV(0.0f, 0.0f, 1.0f)));
	send_gradient_color_uniform(quad_shader, GradientIndex::TRANSPARENT, ColorFrame(0));
	initialize_widget();
	connect_mouse_handlers();
	set_color(ColorFrame());
}

ColorPicker::~ColorPicker()
{
	parent_mb_handler.remove_child(&mb_handler);
	parent_key_handler.remove_child(&key_handler);
}

void ColorPicker::render()
{
	process_mb_down_events();
	rr_wget(*this, BACKGROUND).draw();
	cp_render_gui_back();
	ur_wget(*this, ALPHA_SLIDER).draw();
	ur_wget(*this, ALPHA_SLIDER_CURSOR).draw();
	ur_wget(*this, PREVIEW).draw();
	switch (state)
	{
	case State::GRAPHIC_QUAD:
		ur_wget(*this, GRAPHIC_QUAD).draw();
		ur_wget(*this, GRAPHIC_QUAD_CURSOR).draw();
		ur_wget(*this, GRAPHIC_HUE_SLIDER).draw();
		ur_wget(*this, GRAPHIC_HUE_SLIDER_CURSOR).draw();
		break;
	case State::GRAPHIC_WHEEL:
		ur_wget(*this, GRAPHIC_HUE_WHEEL).draw();
		ur_wget(*this, GRAPHIC_HUE_WHEEL_CURSOR).draw();
		ur_wget(*this, GRAPHIC_VALUE_SLIDER).draw();
		ur_wget(*this, GRAPHIC_VALUE_SLIDER_CURSOR).draw();
		break;
	case State::SLIDER_RGB:
		ur_wget(*this, RGB_R_SLIDER).draw();
		ur_wget(*this, RGB_R_SLIDER_CURSOR).draw();
		ur_wget(*this, RGB_G_SLIDER).draw();
		ur_wget(*this, RGB_G_SLIDER_CURSOR).draw();
		ur_wget(*this, RGB_B_SLIDER).draw();
		ur_wget(*this, RGB_B_SLIDER_CURSOR).draw();
		break;
	case State::SLIDER_HSV:
		ur_wget(*this, HSV_H_SLIDER).draw();
		ur_wget(*this, HSV_H_SLIDER_CURSOR).draw();
		ur_wget(*this, HSV_S_SLIDER).draw();
		ur_wget(*this, HSV_S_SLIDER_CURSOR).draw();
		ur_wget(*this, HSV_V_SLIDER).draw();
		ur_wget(*this, HSV_V_SLIDER_CURSOR).draw();
		break;
	case State::SLIDER_HSL:
		ur_wget(*this, HSL_H_SLIDER).draw();
		ur_wget(*this, HSL_H_SLIDER_CURSOR).draw();
		ur_wget(*this, HSL_S_SLIDER).draw();
		ur_wget(*this, HSL_S_SLIDER_CURSOR).draw();
		ur_wget(*this, HSL_L_SLIDER).draw();
		ur_wget(*this, HSL_L_SLIDER_CURSOR).draw();
		break;
	}
	cp_render_gui_front();
}

void ColorPicker::cp_render_gui_back()
{
	ImGui::SetNextWindowBgAlpha(0);
	auto sz = wp_at(BACKGROUND).transform.scale * Machine.get_app_scale();
	ImGui::SetNextWindowSize(ImVec2(sz.x, sz.y));
	Position pos = gui_center - Position{ 0.5f * sz.x, 0.9f * sz.y };
	ImGui::SetNextWindowPos({ pos.x, pos.y });

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoBackground;
	if (ImGui::Begin("-", nullptr, window_flags))
	{
		State to_state = state;
		imgui_takeover_mb = false;
		imgui_takeover_key = false;
		if (ImGui::BeginTabBar("cp-main-tb"))
		{
			cp_render_tab_button(to_state, last_graphic_state, state == State::GRAPHIC_QUAD || state == State::GRAPHIC_WHEEL, "GRAPHIC");
			cp_render_tab_button(to_state, State::SLIDER_RGB, state == State::SLIDER_RGB, "RGB");
			cp_render_tab_button(to_state, State::SLIDER_HSV, state == State::SLIDER_HSV, "HSV");
			cp_render_tab_button(to_state, State::SLIDER_HSL, state == State::SLIDER_HSL, "HSL");
			ImGui::EndTabBar();
		}
		if (state == State::GRAPHIC_QUAD || state == State::GRAPHIC_WHEEL)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
			if (ImGui::BeginTabBar("cp-sub-tb"))
			{
				cp_render_tab_button(to_state, State::GRAPHIC_QUAD, state == State::GRAPHIC_QUAD, "QUAD");
				cp_render_tab_button(to_state, State::GRAPHIC_WHEEL, state == State::GRAPHIC_WHEEL, "WHEEL");
				ImGui::EndTabBar();
			}
		}
		else if (state == State::SLIDER_RGB)
		{
			if (b_wget(*this, BUTTON_RGB_HEX_CODE).is_pressed(MouseButton::LEFT))
			{
				auto cpos = ImGui::GetCursorPos();
				ImGui::SetNextWindowPos(ImVec2(pos.x + cpos.x, pos.y + cpos.y + 35));
				ImGui::OpenPopup("hex-popup");
			}
			if (ImGui::BeginPopup("hex-popup", ImGuiWindowFlags_NoMove))
			{
				if (Machine.main_window->is_key_pressed(Key::ESCAPE))
				{
					ImGui::CloseCurrentPopup();
				}
				else
				{
					ImGui::Text("RGB hex code");
					ImGui::Text("#");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(90);
					ImGui::InputText("##hex-popup-txtfld", rgb_hex, rgb_hex_size);
					update_rgb_hex();
					imgui_takeover_mb = true;
					imgui_takeover_key = true;
				}
				ImGui::EndPopup();
			}
		}

		float alpha = get_color().alpha;
		float imgui_y = ImGui::GetCursorPosY();
		if (state == State::SLIDER_RGB || state == State::SLIDER_HSV || state == State::SLIDER_HSL)
			imgui_y += 41;
		float imgui_y_1 = imgui_y + 45;
		float imgui_y_2 = imgui_y_1 + 70 * Machine.get_app_scale().y; // TODO slider_sep. put as global constant
		float imgui_y_3 = imgui_y_2 + 70 * Machine.get_app_scale().y;
		float imgui_y_4 = imgui_y_3 + 70 * Machine.get_app_scale().y;
		float imgui_sml_x = 97;
		if (state == State::SLIDER_RGB)
		{
			RGB rgb = get_color().rgb();
			if (txtfld_mode == TextFieldMode::NUMBER)
			{
				bool mod = false;
				int r = rgb.get_pixel_r(), g = rgb.get_pixel_g(), b = rgb.get_pixel_b();
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				mod |= ImGui::InputInt("##it-red", &r, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				mod |= ImGui::InputInt("##it-green", &g, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				mod |= ImGui::InputInt("##it-blue", &b, 5, 10);
				if (mod)
					set_color(ColorFrame(RGB(r, g, b), alpha));
			}
			else if (txtfld_mode == TextFieldMode::PERCENT)
			{
				bool mod = false;
				float r = rgb.r * 100, g = rgb.g * 100, b = rgb.b * 100;
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				mod |= ImGui::InputFloat("##it-red", &r, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				mod |= ImGui::InputFloat("##it-green", &g, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				mod |= ImGui::InputFloat("##it-blue", &b, 5, 10, "%.2f");
				if (mod)
					set_color(RGBA(r * 0.01f, g * 0.01f, b * 0.01f, alpha));
			}
		}
		else if (state == State::SLIDER_HSV)
		{
			HSV hsv = get_color().hsv();
			if (txtfld_mode == TextFieldMode::NUMBER)
			{
				bool mod = false;
				int h = hsv.get_pixel_h(), s = hsv.get_pixel_s(), v = hsv.get_pixel_v();
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				mod |= ImGui::InputInt("##it-hue", &h, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				mod |= ImGui::InputInt("##it-sat", &s, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				mod |= ImGui::InputInt("##it-value", &v, 5, 10);
				if (mod)
					set_color(ColorFrame(HSV(h, s, v), alpha));
			}
			else if (txtfld_mode == TextFieldMode::PERCENT)
			{
				bool mod = false;
				float h = hsv.h * 100, s = hsv.s * 100, v = hsv.v * 100;
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				mod |= ImGui::InputFloat("##it-hue", &h, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				mod |= ImGui::InputFloat("##it-sat", &s, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				mod |= ImGui::InputFloat("##it-value", &v, 5, 10, "%.2f");
				if (mod)
					set_color(HSVA(h * 0.01f, s * 0.01f, v * 0.01f, alpha));
			}
		}
		else if (state == State::SLIDER_HSL)
		{
			HSL hsl = get_color().hsl();
			if (txtfld_mode == TextFieldMode::NUMBER)
			{
				bool mod = false;
				int h = hsl.get_pixel_h(), s = hsl.get_pixel_s(), l = hsl.get_pixel_l();
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				mod |= ImGui::InputInt("##it-hue", &h, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				mod |= ImGui::InputInt("##it-sat", &s, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				mod |= ImGui::InputInt("##it-light", &l, 5, 10);
				if (mod)
					set_color(ColorFrame(HSL(h, s, l), alpha));
			}
			else if (txtfld_mode == TextFieldMode::PERCENT)
			{
				bool mod = false;
				float h = hsl.h * 100, s = hsl.s * 100, l = hsl.l * 100;
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				mod |= ImGui::InputFloat("##it-hue", &h, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				mod |= ImGui::InputFloat("##it-sat", &s, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				mod |= ImGui::InputFloat("##it-light", &l, 5, 10, "%.2f");
				if (mod)
					set_color(HSLA(h * 0.01f, s * 0.01f, l * 0.01f, alpha));
			}
		}
		if (txtfld_mode == TextFieldMode::NUMBER)
		{
			ColorFrame color = get_color();
			int a = color.get_pixel_a();
			ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_4)); 
			if (ImGui::InputInt("##it-alpha", &a, 5, 10))
			{
				color.set_pixel_a(a);
				set_color(color);
			}
		}
		else if (txtfld_mode == TextFieldMode::PERCENT)
		{
			ColorFrame color = get_color();
			float a = color.alpha * 100;
			ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_4));
			if (ImGui::InputFloat("##it-alpha", &a, 5, 10, "%.2f"))
				set_color(ColorFrame(color.rgb(), a * 0.01f));
		}

		set_state(to_state);
		ImGui::End();
	}
}

void ColorPicker::cp_render_gui_front()
{
	if (b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).is_hovered())
	{
		b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().fill_color = RGBA(HSV(0.7f, 0.1f, 0.5f).to_rgb(), 0.9f); // TODO put colors in constexpr in cpp
		b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().update_fill_color().send_buffer();
		Machine.main_window->set_cursor(create_cursor(StandardCursor::HAND));
		Machine.main_window->override_gui_cursor_change(true); // LATER on_press, cancel this highlighting
	}
	else
	{
		b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.9f);
		b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().update_fill_color().send_buffer();
		Machine.main_window->set_cursor(create_cursor(StandardCursor::ARROW)); // TODO prev cursor
		Machine.main_window->override_gui_cursor_change(false);
	}
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).draw();
	if (state == State::SLIDER_RGB)
	{
		if (b_wget(*this, BUTTON_RGB_HEX_CODE).is_hovered())
		{
			b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().fill_color = RGBA(HSV(0.7f, 0.1f, 0.5f).to_rgb(), 0.9f);
			b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().update_fill_color().send_buffer();
			Machine.main_window->set_cursor(create_cursor(StandardCursor::HAND));
			Machine.main_window->override_gui_cursor_change(true);
			// TODO implement "lock" on cursor change, where a void* can be set as owner. When nullptr, imgui can use it.
			// Thus, when classes want to change cursor, they must first query if they own the cursor.
		}
		else
		{
			b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.9f);
			b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().update_fill_color().send_buffer();
			Machine.main_window->set_cursor(create_cursor(StandardCursor::ARROW)); // TODO prev cursor
			Machine.main_window->override_gui_cursor_change(false);
		}
		b_wget(*this, BUTTON_RGB_HEX_CODE).draw();
		tr_wget(*this, TEXT_RED).draw();
		tr_wget(*this, TEXT_GREEN).draw();
		tr_wget(*this, TEXT_BLUE).draw();
	}
	else if (state == State::SLIDER_HSV)
	{
		tr_wget(*this, TEXT_HUE).draw();
		tr_wget(*this, TEXT_SAT).draw();
		tr_wget(*this, TEXT_VALUE).draw();
	}
	else if (state == State::SLIDER_HSL)
	{
		tr_wget(*this, TEXT_HUE).draw();
		tr_wget(*this, TEXT_SAT).draw();
		tr_wget(*this, TEXT_LIGHT).draw();
	}
	tr_wget(*this, TEXT_ALPHA).draw();
}

void ColorPicker::cp_render_tab_button(State& to_state, State state, bool disable, const char* display) const
{
	if (disable)
		ImGui::BeginDisabled();
	if (ImGui::TabItemButton(display))
		to_state = state;
	if (disable)
		ImGui::EndDisabled();
}

static bool is_hex(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static char to_hex(unsigned char num)
{
	return num < 10 ? num + '0' : (num - 10) + 'A';
}

void ColorPicker::update_rgb_hex()
{
	unsigned int hex = 0;
	for (size_t i = 0; i < rgb_hex_size - 1; ++i)
	{
		if (!is_hex(rgb_hex[i]))
		{
			memcpy(rgb_hex, rgb_hex_prev, rgb_hex_size - 1);
			return;
		}
		if (isalpha(rgb_hex[i]))
		{
			rgb_hex[i] = toupper(rgb_hex[i]);
			hex |= ((rgb_hex[i] - 'A') + 10) << (4 * (rgb_hex_size - 2 - i));
		}
		else
			hex |= (rgb_hex[i] - '0') << (4 * (rgb_hex_size - 2 - i));
	}
	set_color(RGB(hex));
}

void ColorPicker::set_state(State _state)
{
	if (_state != state)
	{
		if (_state == State::GRAPHIC_QUAD)
			last_graphic_state = State::GRAPHIC_QUAD;
		else if (_state == State::GRAPHIC_WHEEL)
			last_graphic_state = State::GRAPHIC_WHEEL;
		else if (state == State::GRAPHIC_QUAD || state == State::GRAPHIC_WHEEL)
			last_graphic_state = state;

		release_cursor();
		ColorFrame pre_color = get_color();
		state = _state;
		set_color(pre_color);
	}
}

void ColorPicker::send_vp()
{
	Uniforms::send_matrix3(quad_shader, "u_VP", *vp);
	Uniforms::send_matrix3(linear_hue_shader, "u_VP", *vp);
	Uniforms::send_matrix3(hue_wheel_w_shader, "u_VP", *vp);
	Uniforms::send_matrix3(linear_lightness_shader, "u_VP", *vp);
	Uniforms::send_matrix3(circle_cursor_shader, "u_VP", *vp);
	Uniforms::send_matrix3(round_rect_shader, "u_VP", *vp);
	sync_cp_widget_with_vp();
}

void ColorPicker::initialize_widget()
{
	// LATER put in ColorPicker struct data member for access in cp_render_gui()
	// ---------- COMMON CONSTANTS ----------

	const float graphic_x = -15;
	const float graphic_y = 40;
	const float graphic_sx = 180;
	const float graphic_sy = 180;
	const float g_slider_x = 97.5f;
	const float g_slider_y = graphic_y;
	const float g_slider_w = 15;
	const float g_slider_h = 180;

	const float slider_sep = 70;
	const float slider_x = 0;
	const float slider1_y = graphic_y + graphic_sy * 0.5f;
	const float slider2_y = slider1_y - slider_sep;
	const float slider3_y = slider2_y - slider_sep;
	const float slider4_y = slider3_y - slider_sep;
	const float slider_w = 200;
	const float slider_h = 20;
	
	const float preview_x = 0;
	const float preview_y = slider4_y - 60;
	const float preview_w = 80;
	const float preview_h = 40;

	const float left_text_x = -98;
	const float text_sep = 70;
	const float text1_y = slider1_y - slider_h * 1.5f;
	const float text2_y = text1_y - text_sep;
	const float text3_y = text2_y - text_sep;
	const float text4_y = text3_y - text_sep;
	
	const float button_rgb_hex_code_x = -90;
	const float button_switch_txtfld_mode_x = 102;
	const float button_y = 160;
	const float button_scale = 0.9f;
	const float button_rgb_hex_code_w = 50;
	const float button_switch_txtfld_mode_w = 25;
	const float button_h = 30;

	// ---------- GRAPHIC QUAD ----------

	assign_widget(this, GRAPHIC_QUAD, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, GRAPHIC_QUAD_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));
	assign_widget(this, GRAPHIC_HUE_SLIDER, new WP_UnitRenderable(&linear_hue_shader));
	assign_widget(this, GRAPHIC_HUE_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));

	setup_rect_uvs(GRAPHIC_QUAD);
	setup_gradient(GRAPHIC_QUAD, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::WHITE, (GLint)GradientIndex::GRAPHIC_QUAD);
	send_graphic_quad_hue_to_uniform(0.0f);
	setup_circle_cursor(GRAPHIC_QUAD_CURSOR);
	orient_progress_slider(GRAPHIC_HUE_SLIDER, Cardinal::UP);
	setup_circle_cursor(GRAPHIC_HUE_SLIDER_CURSOR);

	wp_at(GRAPHIC_QUAD).transform.position.x = graphic_x;
	wp_at(GRAPHIC_QUAD).transform.position.y = graphic_y;
	wp_at(GRAPHIC_QUAD).transform.scale = { graphic_sx, graphic_sy };
	wp_at(GRAPHIC_QUAD_CURSOR).transform.position = { wp_at(GRAPHIC_QUAD).top(), wp_at(GRAPHIC_QUAD).right() };
	wp_at(GRAPHIC_HUE_SLIDER).transform.position.x = g_slider_x;
	wp_at(GRAPHIC_HUE_SLIDER).transform.position.y = g_slider_y;
	wp_at(GRAPHIC_HUE_SLIDER).transform.scale = { g_slider_w, g_slider_h };
	wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position = { wp_at(GRAPHIC_HUE_SLIDER).center_x(), wp_at(GRAPHIC_HUE_SLIDER).bottom() };

	// ---------- GRAPHIC WHEEL ----------

	assign_widget(this, GRAPHIC_HUE_WHEEL, new WP_UnitRenderable(&hue_wheel_w_shader));
	assign_widget(this, GRAPHIC_HUE_WHEEL_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));
	assign_widget(this, GRAPHIC_VALUE_SLIDER, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, GRAPHIC_VALUE_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));

	setup_rect_uvs(GRAPHIC_HUE_WHEEL);
	send_graphic_wheel_value_to_uniform(1.0f);
	setup_circle_cursor(GRAPHIC_HUE_WHEEL_CURSOR);
	setup_rect_uvs(GRAPHIC_VALUE_SLIDER);
	setup_gradient(GRAPHIC_VALUE_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::GRAPHIC_VALUE_SLIDER, (GLint)GradientIndex::GRAPHIC_VALUE_SLIDER);
	send_graphic_value_slider_hue_and_sat_to_uniform(0.0f, 0.0f);
	setup_circle_cursor(GRAPHIC_VALUE_SLIDER_CURSOR);
	set_circle_cursor_value(GRAPHIC_VALUE_SLIDER_CURSOR, 0.0f);

	wp_at(GRAPHIC_HUE_WHEEL).transform.position.x = graphic_x;
	wp_at(GRAPHIC_HUE_WHEEL).transform.position.y = graphic_y;
	wp_at(GRAPHIC_HUE_WHEEL).transform.scale = { graphic_sx, graphic_sy };
	wp_at(GRAPHIC_VALUE_SLIDER).transform.position.x = g_slider_x;
	wp_at(GRAPHIC_VALUE_SLIDER).transform.position.y = g_slider_y;
	wp_at(GRAPHIC_VALUE_SLIDER).transform.scale = { g_slider_w, g_slider_h };
	wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position = { wp_at(GRAPHIC_VALUE_SLIDER).center_x(), wp_at(GRAPHIC_VALUE_SLIDER).top() };

	// ---------- RGB SLIDERS ----------

	assign_widget(this, RGB_R_SLIDER, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, RGB_R_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));
	assign_widget(this, RGB_G_SLIDER, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, RGB_G_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));
	assign_widget(this, RGB_B_SLIDER, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, RGB_B_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));

	setup_rect_uvs(RGB_R_SLIDER);
	setup_circle_cursor(RGB_R_SLIDER_CURSOR);
	setup_gradient(RGB_R_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_R_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_R_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::RGB_R_SLIDER, RGB(0xFF0000));
	setup_rect_uvs(RGB_G_SLIDER);
	setup_circle_cursor(RGB_G_SLIDER_CURSOR);
	setup_gradient(RGB_G_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_G_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_G_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::RGB_G_SLIDER, RGB(0x00FF00));
	setup_rect_uvs(RGB_B_SLIDER);
	setup_circle_cursor(RGB_B_SLIDER_CURSOR);
	setup_gradient(RGB_B_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_B_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_B_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::RGB_B_SLIDER, RGB(0x0000FF));

	wp_at(RGB_R_SLIDER).transform.position.x = slider_x;
	wp_at(RGB_R_SLIDER).transform.position.y = slider1_y;
	wp_at(RGB_R_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(RGB_R_SLIDER_CURSOR).transform.position = { wp_at(RGB_R_SLIDER).right(), wp_at(RGB_R_SLIDER).center_y() };
	wp_at(RGB_G_SLIDER).transform.position.x = slider_x;
	wp_at(RGB_G_SLIDER).transform.position.y = slider2_y;
	wp_at(RGB_G_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(RGB_G_SLIDER_CURSOR).transform.position = { wp_at(RGB_G_SLIDER).right(), wp_at(RGB_G_SLIDER).center_y() };
	wp_at(RGB_B_SLIDER).transform.position.x = slider_x;
	wp_at(RGB_B_SLIDER).transform.position.y = slider3_y;
	wp_at(RGB_B_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(RGB_B_SLIDER_CURSOR).transform.position = { wp_at(RGB_B_SLIDER).right(), wp_at(RGB_B_SLIDER).center_y() };

	// ---------- HSV SLIDERS ----------

	assign_widget(this, HSV_H_SLIDER, new WP_UnitRenderable(&linear_hue_shader));
	assign_widget(this, HSV_H_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));
	assign_widget(this, HSV_S_SLIDER, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, HSV_S_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));
	assign_widget(this, HSV_V_SLIDER, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, HSV_V_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));

	orient_progress_slider(HSV_H_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(HSV_H_SLIDER_CURSOR);
	setup_rect_uvs(HSV_S_SLIDER);
	setup_circle_cursor(HSV_S_SLIDER_CURSOR);
	setup_gradient(HSV_S_SLIDER, (GLint)GradientIndex::HSV_S_SLIDER_ZERO, (GLint)GradientIndex::HSV_S_SLIDER_ONE, (GLint)GradientIndex::HSV_S_SLIDER_ZERO, (GLint)GradientIndex::HSV_S_SLIDER_ONE);
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ZERO, HSV(0.0f, 0.0f, 1.0f));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ONE, HSV(0.0f, 1.0f, 1.0f));
	setup_rect_uvs(HSV_V_SLIDER);
	setup_circle_cursor(HSV_V_SLIDER_CURSOR);
	setup_gradient(HSV_V_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::HSV_V_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::HSV_V_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_V_SLIDER, HSV(0.0f, 1.0f, 1.0f));

	wp_at(HSV_H_SLIDER).transform.position.x = slider_x;
	wp_at(HSV_H_SLIDER).transform.position.y = slider1_y;
	wp_at(HSV_H_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(HSV_H_SLIDER_CURSOR).transform.position = { wp_at(HSV_H_SLIDER).right(), wp_at(HSV_H_SLIDER).center_y() };
	wp_at(HSV_S_SLIDER).transform.position.x = slider_x;
	wp_at(HSV_S_SLIDER).transform.position.y = slider2_y;
	wp_at(HSV_S_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(HSV_S_SLIDER_CURSOR).transform.position = { wp_at(HSV_S_SLIDER).right(), wp_at(HSV_S_SLIDER).center_y() };
	wp_at(HSV_V_SLIDER).transform.position.x = slider_x;
	wp_at(HSV_V_SLIDER).transform.position.y = slider3_y;
	wp_at(HSV_V_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(HSV_V_SLIDER_CURSOR).transform.position = { wp_at(HSV_V_SLIDER).right(), wp_at(HSV_V_SLIDER).center_y() };

	// ---------- HSL SLIDERS ----------

	assign_widget(this, HSL_H_SLIDER, new WP_UnitRenderable(&linear_hue_shader));
	assign_widget(this, HSL_H_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));
	assign_widget(this, HSL_S_SLIDER, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, HSL_S_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));
	assign_widget(this, HSL_L_SLIDER, new WP_UnitRenderable(&linear_lightness_shader));
	assign_widget(this, HSL_L_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));

	orient_progress_slider(HSL_H_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(HSL_H_SLIDER_CURSOR);
	setup_rect_uvs(HSL_S_SLIDER);
	setup_circle_cursor(HSL_S_SLIDER_CURSOR);
	setup_gradient(HSL_S_SLIDER, (GLint)GradientIndex::HSL_S_SLIDER_ZERO, (GLint)GradientIndex::HSL_S_SLIDER_ONE, (GLint)GradientIndex::HSL_S_SLIDER_ZERO, (GLint)GradientIndex::HSL_S_SLIDER_ONE);
	send_gradient_color_uniform(quad_shader, GradientIndex::HSL_S_SLIDER_ZERO, HSL(0.0f, 0.0f, 0.5f));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSL_S_SLIDER_ONE, HSL(0.0f, 1.0f, 0.5f));
	orient_progress_slider(HSL_L_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(HSL_L_SLIDER_CURSOR);
	
	wp_at(HSL_H_SLIDER).transform.position.x = slider_x;
	wp_at(HSL_H_SLIDER).transform.position.y = slider1_y;
	wp_at(HSL_H_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(HSL_H_SLIDER_CURSOR).transform.position = { wp_at(HSL_H_SLIDER).right(), wp_at(HSL_H_SLIDER).center_y() };
	wp_at(HSL_S_SLIDER).transform.position.x = slider_x;
	wp_at(HSL_S_SLIDER).transform.position.y = slider2_y;
	wp_at(HSL_S_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(HSL_S_SLIDER_CURSOR).transform.position = { wp_at(HSL_S_SLIDER).right(), wp_at(HSL_S_SLIDER).center_y() };
	wp_at(HSL_L_SLIDER).transform.position.x = slider_x;
	wp_at(HSL_L_SLIDER).transform.position.y = slider3_y;
	wp_at(HSL_L_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(HSL_L_SLIDER_CURSOR).transform.position = { wp_at(HSL_L_SLIDER).right(), wp_at(HSL_L_SLIDER).center_y() };

	// ---------- ALPHA SLIDER ----------
	
	assign_widget(this, ALPHA_SLIDER, new WP_UnitRenderable(&quad_shader));
	assign_widget(this, ALPHA_SLIDER_CURSOR, new WP_UnitRenderable(&circle_cursor_shader));

	setup_rect_uvs(ALPHA_SLIDER);
	setup_circle_cursor(ALPHA_SLIDER_CURSOR);
	setup_gradient(ALPHA_SLIDER, (GLint)GradientIndex::TRANSPARENT, (GLint)GradientIndex::ALPHA_SLIDER, (GLint)GradientIndex::TRANSPARENT, (GLint)GradientIndex::ALPHA_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::ALPHA_SLIDER, ColorFrame());

	wp_at(ALPHA_SLIDER).transform.position.x = slider_x;
	wp_at(ALPHA_SLIDER).transform.position.y = slider4_y;
	wp_at(ALPHA_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(ALPHA_SLIDER_CURSOR).transform.position = { wp_at(ALPHA_SLIDER).right(), wp_at(ALPHA_SLIDER).center_y() };

	// ---------- PREVIEW ----------
	
	assign_widget(this, PREVIEW, new WP_UnitRenderable(&quad_shader));
	setup_rect_uvs(PREVIEW);
	setup_gradient(PREVIEW, (GLint)GradientIndex::PREVIEW, (GLint)GradientIndex::PREVIEW, (GLint)GradientIndex::PREVIEW, (GLint)GradientIndex::PREVIEW);
	send_gradient_color_uniform(quad_shader, GradientIndex::PREVIEW, ColorFrame());
	wp_at(PREVIEW).transform.position.x = preview_x;
	wp_at(PREVIEW).transform.position.y = preview_y;
	wp_at(PREVIEW).transform.scale = { preview_w, preview_h };
	wp_at(PREVIEW).pivot.y = 1;

	// ---------- BACKGROUND ----------

	assign_widget(this, BACKGROUND, new RoundRect(&round_rect_shader));
	rr_wget(*this, BACKGROUND).thickness = 0.5f;
	rr_wget(*this, BACKGROUND).corner_radius = 10;
	rr_wget(*this, BACKGROUND).border_color = RGBA(HSV(0.7f, 0.5f, 0.5f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).update_all();

	// ---------- TEXT ----------

	assign_widget(this, TEXT_ALPHA, new TextRender(*Fonts::label_regular, 18, "Alpha"));
	wp_at(TEXT_ALPHA).pivot.y = 0.5f;
	tr_wget(*this, TEXT_ALPHA).setup_renderable();
	wp_at(TEXT_ALPHA).transform.position.x = left_text_x;
	wp_at(TEXT_ALPHA).transform.position.y = text4_y;

	assign_widget(this, TEXT_RED, new TextRender(*Fonts::label_regular, 18, "Red"));
	wp_at(TEXT_RED).pivot.y = 0.5f;
	tr_wget(*this, TEXT_RED).setup_renderable();
	wp_at(TEXT_RED).transform.position.x = left_text_x;
	wp_at(TEXT_RED).transform.position.y = text1_y;

	assign_widget(this, TEXT_GREEN, new TextRender(*Fonts::label_regular, 18, "Green"));
	wp_at(TEXT_GREEN).pivot.y = 0.5f;
	tr_wget(*this, TEXT_GREEN).setup_renderable();
	wp_at(TEXT_GREEN).transform.position.x = left_text_x;
	wp_at(TEXT_GREEN).transform.position.y = text2_y;

	assign_widget(this, TEXT_BLUE, new TextRender(*Fonts::label_regular, 18, "Blue"));
	wp_at(TEXT_BLUE).pivot.y = 0.5f;
	tr_wget(*this, TEXT_BLUE).setup_renderable();
	wp_at(TEXT_BLUE).transform.position.x = left_text_x;
	wp_at(TEXT_BLUE).transform.position.y = text3_y;

	assign_widget(this, TEXT_HUE, new TextRender(*Fonts::label_regular, 18, "Hue"));
	wp_at(TEXT_HUE).pivot.y = 0.5f;
	tr_wget(*this, TEXT_HUE).setup_renderable();
	wp_at(TEXT_HUE).transform.position.x = left_text_x;
	wp_at(TEXT_HUE).transform.position.y = text1_y;

	assign_widget(this, TEXT_SAT, new TextRender(*Fonts::label_regular, 18, "Sat"));
	wp_at(TEXT_SAT).pivot.y = 0.5f;
	tr_wget(*this, TEXT_SAT).setup_renderable();
	wp_at(TEXT_SAT).transform.position.x = left_text_x;
	wp_at(TEXT_SAT).transform.position.y = text2_y;

	assign_widget(this, TEXT_VALUE, new TextRender(*Fonts::label_regular, 18, "Value"));
	wp_at(TEXT_VALUE).pivot.y = 0.5f;
	tr_wget(*this, TEXT_VALUE).setup_renderable();
	wp_at(TEXT_VALUE).transform.position.x = left_text_x;
	wp_at(TEXT_VALUE).transform.position.y = text3_y;

	assign_widget(this, TEXT_LIGHT, new TextRender(*Fonts::label_regular, 18, "Light"));
	wp_at(TEXT_LIGHT).pivot.y = 0.5f;
	tr_wget(*this, TEXT_LIGHT).setup_renderable();
	wp_at(TEXT_LIGHT).transform.position.x = left_text_x;
	wp_at(TEXT_LIGHT).transform.position.y = text3_y;
	
	// ---------- BUTTONS ----------

	assign_widget(this, BUTTON_RGB_HEX_CODE, new Button(vp, {}, *Fonts::label_regular, 18, &round_rect_shader, mb_handler, "HEX"));
	wp_at(BUTTON_RGB_HEX_CODE).transform.position = { button_rgb_hex_code_x, button_y };
	wp_at(BUTTON_RGB_HEX_CODE).transform.scale = Scale(button_scale);
	b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().self.transform.scale = { button_rgb_hex_code_w, button_h };
	b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().thickness = 0.5f;
	b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().corner_radius = 5;
	b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().border_color = RGBA(HSV(0.7f, 0.5f, 0.2f).to_rgb(), 1.0f);
	b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.9f);
	b_wget(*this, BUTTON_RGB_HEX_CODE).bkg().update_all();

	assign_widget(this, BUTTON_SWITCH_TXTFLD_MODE, new Button(vp, {}, *Fonts::label_bolditalic, 18, &round_rect_shader, mb_handler, "#"));
	wp_at(BUTTON_SWITCH_TXTFLD_MODE).transform.position = { button_switch_txtfld_mode_x, button_y };
	wp_at(BUTTON_SWITCH_TXTFLD_MODE).transform.scale = Scale(button_scale);
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().self.transform.scale = { button_switch_txtfld_mode_w, button_h };
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().thickness = 0.5f;
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().corner_radius = 5;
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().border_color = RGBA(HSV(0.7f, 0.5f, 0.2f).to_rgb(), 1.0f);
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.9f);
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).bkg().update_all();
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).on_release = [this](const MouseButtonEvent& mb, Position) {
		if (mb.button == MouseButton::LEFT)
		{
			if (txtfld_mode == TextFieldMode::NUMBER)
			{
				txtfld_mode = TextFieldMode::PERCENT;
				b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).text().set_text("%");
			}
			else if (txtfld_mode == TextFieldMode::PERCENT)
			{
				txtfld_mode = TextFieldMode::NUMBER;
				b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).text().set_text("#");
			}
		}
		};
}

void ColorPicker::connect_mouse_handlers()
{
	parent_mb_handler.children.push_back(&mb_handler);
	mb_handler.callback = [this](const MouseButtonEvent& mb) {
		if (mb.action == IAction::PRESS && mb.button == MouseButton::LEFT)
		{
			if (current_widget_control < 0)
			{
				Position local_cursor_pos = local_of(Machine.cursor_world_coordinates(glm::inverse(*vp)));
				if (wp_at(ALPHA_SLIDER).contains_point(local_cursor_pos))
				{
					take_over_cursor();
					current_widget_control = ALPHA_SLIDER_CURSOR;
					mb.consumed = true;
					mouse_handler_horizontal_slider(ALPHA_SLIDER, ALPHA_SLIDER_CURSOR, local_cursor_pos);
				}
				else if (state == State::GRAPHIC_QUAD)
				{
					if (wp_at(GRAPHIC_QUAD).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_QUAD_CURSOR;
						mb.consumed = true;
						mouse_handler_graphic_quad(local_cursor_pos);
					}
					else if (wp_at(GRAPHIC_HUE_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_HUE_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_vertical_slider(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR, local_cursor_pos);
					}
				}
				else if (state == State::GRAPHIC_WHEEL)
				{
					if (wp_at(GRAPHIC_HUE_WHEEL).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_HUE_WHEEL_CURSOR;
						mb.consumed = true;
						mouse_handler_graphic_hue_wheel(local_cursor_pos);
					}
					else if (wp_at(GRAPHIC_VALUE_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_VALUE_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_vertical_slider(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR, local_cursor_pos);
					}
				}
				else if (state == State::SLIDER_RGB)
				{
					if (wp_at(RGB_R_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_R_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(RGB_R_SLIDER, RGB_R_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(RGB_G_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_G_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(RGB_G_SLIDER, RGB_G_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(RGB_B_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_B_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(RGB_B_SLIDER, RGB_B_SLIDER_CURSOR, local_cursor_pos);
					}
				}
				else if (state == State::SLIDER_HSV)
				{
					if (wp_at(HSV_H_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_H_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(HSV_S_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_S_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(HSV_V_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_V_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR, local_cursor_pos);
					}
				}
				else if (state == State::SLIDER_HSL)
				{
					if (wp_at(HSL_H_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_H_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(HSL_S_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_S_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(HSL_L_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_L_SLIDER_CURSOR;
						mb.consumed = true;
						mouse_handler_horizontal_slider(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR, local_cursor_pos);
					}
				}

				if (current_widget_control >= 0)
					update_display_colors();
			}
		}
		else if (mb.action == IAction::RELEASE)
		{
			release_cursor();
			mb.consumed = true;
		}
	};
	mb_handler.children.push_back(&imgui_mb_handler);
	imgui_mb_handler.callback = [this](const MouseButtonEvent& mb) {
		if (imgui_takeover_mb)
			mb.consumed = true;
		};

	parent_key_handler.children.push_back(&key_handler);
	key_handler.callback = [this](const KeyEvent& key) {
		if (imgui_takeover_key)
			key.consumed = true;
		};
}

void ColorPicker::process_mb_down_events()
{
	if (!Machine.main_window->is_mouse_button_pressed(MouseButton::LEFT))
		return;

	Position local_cursor_pos = self.transform.get_relative_pos(Machine.palette_cursor_world_pos());
	if (current_widget_control == ALPHA_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(ALPHA_SLIDER, ALPHA_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == GRAPHIC_QUAD_CURSOR)
		mouse_handler_graphic_quad(local_cursor_pos);
	else if (current_widget_control == GRAPHIC_HUE_SLIDER_CURSOR)
		mouse_handler_vertical_slider(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == GRAPHIC_HUE_WHEEL_CURSOR)
		mouse_handler_graphic_hue_wheel(local_cursor_pos);
	else if (current_widget_control == GRAPHIC_VALUE_SLIDER_CURSOR)
		mouse_handler_vertical_slider(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == RGB_R_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(RGB_R_SLIDER, RGB_R_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == RGB_G_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(RGB_G_SLIDER, RGB_G_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == RGB_B_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(RGB_B_SLIDER, RGB_B_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == HSV_H_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == HSV_S_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == HSV_V_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == HSL_H_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == HSL_S_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR, local_cursor_pos);
	else if (current_widget_control == HSL_L_SLIDER_CURSOR)
		mouse_handler_horizontal_slider(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR, local_cursor_pos);

	if (current_widget_control >= 0)
		update_display_colors();
}

void ColorPicker::take_over_cursor() const
{
	//Machine.main_window->set_mouse_mode(MouseMode::VIRTUAL);
	Machine.main_window->override_gui_cursor_change(true);
	Machine.main_window->set_cursor(create_cursor(StandardCursor::CROSSHAIR));
}

void ColorPicker::release_cursor()
{
	if (current_widget_control >= 0)
	{
		//Machine.main_window->set_mouse_mode(MouseMode::VISIBLE);
		Machine.main_window->override_gui_cursor_change(false);
		Machine.main_window->set_cursor(create_cursor(StandardCursor::ARROW));
		current_widget_control = -1;
	}
}

ColorFrame ColorPicker::get_color() const
{
	ColorFrame color(slider_normal_x(ALPHA_SLIDER, ALPHA_SLIDER_CURSOR));
	if (state == State::GRAPHIC_QUAD)
	{
		glm::vec2 sv = get_graphic_quad_sat_and_value();
		color.set_hsv(HSV(slider_normal_y(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR), sv[0], sv[1]));
	}
	else if (state == State::GRAPHIC_WHEEL)
	{
		glm::vec2 hs = get_graphic_wheel_hue_and_sat();
		color.set_hsv(HSV(hs[0], hs[1], slider_normal_y(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR)));
	}
	else if (state == State::SLIDER_RGB)
	{
		float r = slider_normal_x(RGB_R_SLIDER, RGB_R_SLIDER_CURSOR);
		float g = slider_normal_x(RGB_G_SLIDER, RGB_G_SLIDER_CURSOR);
		float b = slider_normal_x(RGB_B_SLIDER, RGB_B_SLIDER_CURSOR);
		color.set_rgb(RGB(r, g, b));
	}
	else if (state == State::SLIDER_HSV)
	{
		float h = slider_normal_x(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR);
		float s = slider_normal_x(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR);
		float v = slider_normal_x(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR);
		color.set_hsv(HSV(h, s, v));
	}
	else if (state == State::SLIDER_HSL)
	{
		float h = slider_normal_x(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR);
		float s = slider_normal_x(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR);
		float l = slider_normal_x(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR);
		color.set_hsl(HSL(h, s, l));
	}
	return color;
}

void ColorPicker::set_color(ColorFrame color)
{
	move_slider_cursor_x_relative(ALPHA_SLIDER, ALPHA_SLIDER_CURSOR, color.alpha);
	sync_single_cp_widget_transform_ur(ALPHA_SLIDER_CURSOR);
	if (state == State::GRAPHIC_QUAD)
	{
		HSV hsv = color.hsv();
		wp_at(GRAPHIC_QUAD_CURSOR).transform.position.x = wp_at(GRAPHIC_QUAD).interp_x(hsv.s);
		wp_at(GRAPHIC_QUAD_CURSOR).transform.position.y = wp_at(GRAPHIC_QUAD).interp_y(hsv.v);
		move_slider_cursor_y_relative(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR, hsv.h);
		sync_single_cp_widget_transform_ur(GRAPHIC_QUAD_CURSOR);
		sync_single_cp_widget_transform_ur(GRAPHIC_HUE_SLIDER_CURSOR);
	}
	else if (state == State::GRAPHIC_WHEEL)
	{
		HSV hsv = color.hsv();
		float x = hsv.s * glm::cos(glm::tau<float>() * hsv.h);
		float y = -hsv.s * glm::sin(glm::tau<float>() * hsv.h);
		wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position.x = wp_at(GRAPHIC_HUE_WHEEL).interp_x(0.5f * (x + 1));
		wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position.y = wp_at(GRAPHIC_HUE_WHEEL).interp_y(0.5f * (y + 1));
		move_slider_cursor_y_relative(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR, hsv.v);
		sync_single_cp_widget_transform_ur(GRAPHIC_HUE_WHEEL_CURSOR);
		sync_single_cp_widget_transform_ur(GRAPHIC_VALUE_SLIDER_CURSOR);
	}
	else if (state == State::SLIDER_RGB)
	{
		RGB rgb = color.rgb();
		move_slider_cursor_x_relative(RGB_R_SLIDER, RGB_R_SLIDER_CURSOR, rgb.r);
		move_slider_cursor_x_relative(RGB_G_SLIDER, RGB_G_SLIDER_CURSOR, rgb.g);
		move_slider_cursor_x_relative(RGB_B_SLIDER, RGB_B_SLIDER_CURSOR, rgb.b);
		sync_single_cp_widget_transform_ur(RGB_R_SLIDER_CURSOR);
		sync_single_cp_widget_transform_ur(RGB_G_SLIDER_CURSOR);
		sync_single_cp_widget_transform_ur(RGB_B_SLIDER_CURSOR);
	}
	else if (state == State::SLIDER_HSV)
	{
		HSV hsv = color.hsv();
		move_slider_cursor_x_relative(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR, hsv.h);
		move_slider_cursor_x_relative(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR, hsv.s);
		move_slider_cursor_x_relative(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR, hsv.v);
		sync_single_cp_widget_transform_ur(HSV_H_SLIDER_CURSOR);
		sync_single_cp_widget_transform_ur(HSV_S_SLIDER_CURSOR);
		sync_single_cp_widget_transform_ur(HSV_V_SLIDER_CURSOR);
	}
	else if (state == State::SLIDER_HSL)
	{
		HSL hsl = color.hsl();
		move_slider_cursor_x_relative(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR, hsl.h);
		move_slider_cursor_x_relative(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR, hsl.s);
		move_slider_cursor_x_relative(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR, hsl.l);
		sync_single_cp_widget_transform_ur(HSL_H_SLIDER_CURSOR);
		sync_single_cp_widget_transform_ur(HSL_S_SLIDER_CURSOR);
		sync_single_cp_widget_transform_ur(HSL_L_SLIDER_CURSOR);
	}
	update_display_colors();
}

void ColorPicker::set_size(Scale size, bool sync)
{
	wp_at(BACKGROUND).transform.scale = size;
	rr_wget(*this, BACKGROUND).update_transform();
	if (sync)
		sync_cp_widget_with_vp();
}

void ColorPicker::set_position(Position world_pos)
{
	self.transform.position = world_pos;
	gui_center = Machine.to_screen_coordinates(world_pos, *vp);
}

void ColorPicker::mouse_handler_graphic_quad(Position local_cursor_pos)
{
	wp_at(GRAPHIC_QUAD_CURSOR).transform.position = wp_at(GRAPHIC_QUAD).clamp_point(local_cursor_pos);
	sync_single_cp_widget_transform_ur(GRAPHIC_QUAD_CURSOR);
}

void ColorPicker::mouse_handler_graphic_hue_wheel(Position local_cursor_pos)
{
	wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position = wp_at(GRAPHIC_HUE_WHEEL).clamp_point_in_ellipse(local_cursor_pos);
	sync_single_cp_widget_transform_ur(GRAPHIC_HUE_WHEEL_CURSOR);
}

void ColorPicker::mouse_handler_horizontal_slider(size_t slider, size_t cursor, Position local_cursor_pos)
{
	move_slider_cursor_x_absolute(slider, cursor, local_cursor_pos.x);
	sync_single_cp_widget_transform_ur(cursor);
}

void ColorPicker::mouse_handler_vertical_slider(size_t slider, size_t cursor, Position local_cursor_pos)
{
	move_slider_cursor_y_absolute(slider, cursor, local_cursor_pos.y);
	sync_single_cp_widget_transform_ur(cursor);
}

void ColorPicker::move_slider_cursor_x_absolute(size_t control, size_t cursor, float absolute)
{
	wp_at(cursor).transform.position.x = wp_at(control).clamp_x(absolute);
	wp_at(cursor).transform.position.y = wp_at(control).center_y();
}

void ColorPicker::move_slider_cursor_x_relative(size_t control, size_t cursor, float relative)
{
	wp_at(cursor).transform.position.x = wp_at(control).interp_x(relative);
	wp_at(cursor).transform.position.y = wp_at(control).center_y();
}

void ColorPicker::move_slider_cursor_y_absolute(size_t control, size_t cursor, float absolute)
{
	wp_at(cursor).transform.position.x = wp_at(control).center_x();
	wp_at(cursor).transform.position.y = wp_at(control).clamp_y(absolute);
}

void ColorPicker::move_slider_cursor_y_relative(size_t control, size_t cursor, float relative)
{
	wp_at(cursor).transform.position.x = wp_at(control).center_x();
	wp_at(cursor).transform.position.y = wp_at(control).interp_y(relative);
}

void ColorPicker::update_display_colors()
{
	ColorFrame color = get_color();
	// preview
	send_gradient_color_uniform(quad_shader, GradientIndex::PREVIEW, color);
	// alpha
	send_gradient_color_uniform(quad_shader, GradientIndex::ALPHA_SLIDER, ColorFrame(color.rgb()));
	set_circle_cursor_value(ALPHA_SLIDER_CURSOR, contrast_wb_value_complex_hsva(color.hsva()));
	send_cpwc_buffer(ALPHA_SLIDER_CURSOR);
	if (state == State::GRAPHIC_QUAD)
	{
		HSV hsv = color.hsv();
		set_circle_cursor_value(GRAPHIC_QUAD_CURSOR, contrast_wb_value_complex_hsv(hsv));
		send_cpwc_buffer(GRAPHIC_QUAD_CURSOR);
		send_graphic_quad_hue_to_uniform(hsv.h);
		set_circle_cursor_value(GRAPHIC_HUE_SLIDER_CURSOR, contrast_wb_value_simple_hue(hsv.h));
		send_cpwc_buffer(GRAPHIC_HUE_SLIDER_CURSOR);
	}
	else if (state == State::GRAPHIC_WHEEL)
	{
		HSV hsv = color.hsv();
		set_circle_cursor_value(GRAPHIC_HUE_WHEEL_CURSOR, contrast_wb_value_simple_hue_and_sat(hsv.h, hsv.s));
		send_cpwc_buffer(GRAPHIC_HUE_WHEEL_CURSOR);
		send_graphic_value_slider_hue_and_sat_to_uniform(hsv.h, hsv.s);
		set_circle_cursor_value(GRAPHIC_VALUE_SLIDER_CURSOR, contrast_wb_value_simple_hue_and_value(hsv.h, hsv.v)); // LATER better contrast functions
		send_graphic_wheel_value_to_uniform(hsv.v);
		send_cpwc_buffer(GRAPHIC_VALUE_SLIDER_CURSOR);
	}
	else if (state == State::SLIDER_RGB)
	{
		RGB rgb = color.rgb();
		// LATER cursor contrast ? possibly not necessary
		// hex
		rgb_hex[0] = to_hex(rgb.get_pixel_r() >> 4);
		rgb_hex[1] = to_hex(rgb.get_pixel_r() & 0xF);
		rgb_hex[2] = to_hex(rgb.get_pixel_g() >> 4);
		rgb_hex[3] = to_hex(rgb.get_pixel_g() & 0xF);
		rgb_hex[4] = to_hex(rgb.get_pixel_b() >> 4);
		rgb_hex[5] = to_hex(rgb.get_pixel_b() & 0xF);
		memcpy(rgb_hex_prev, rgb_hex, rgb_hex_size - 1);
	}
	else if (state == State::SLIDER_HSV)
	{
		HSV hsv = color.hsv();
		set_circle_cursor_value(HSV_H_SLIDER_CURSOR, contrast_wb_value_simple_hue(hsv.h));
		set_circle_cursor_value(HSV_S_SLIDER_CURSOR, contrast_wb_value_complex_hsv(hsv));
		set_circle_cursor_value(HSV_V_SLIDER_CURSOR, contrast_wb_value_simple_hue_and_value(hsv.h, hsv.v));
		send_slider_hsv_hue_and_value_to_uniform(hsv.h, hsv.v);
		send_cpwc_buffer(HSV_H_SLIDER_CURSOR);
		send_cpwc_buffer(HSV_S_SLIDER_CURSOR);
		send_cpwc_buffer(HSV_V_SLIDER_CURSOR);
	}
	else if (state == State::SLIDER_HSL)
	{
		HSL hsl = color.hsl();
		set_circle_cursor_value(HSL_H_SLIDER_CURSOR, contrast_wb_value_simple_hue(slider_normal_x(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR)));
		set_circle_cursor_value(HSL_S_SLIDER_CURSOR, contrast_wb_value_complex_hsl(hsl));
		set_circle_cursor_value(HSL_L_SLIDER_CURSOR, contrast_wb_value_simple_hue_and_lightness(hsl.h, hsl.l));
		send_slider_hsl_hue_and_lightness_to_uniform(hsl.h, hsl.l);
		send_cpwc_buffer(HSL_H_SLIDER_CURSOR);
		send_cpwc_buffer(HSL_S_SLIDER_CURSOR);
		send_cpwc_buffer(HSL_L_SLIDER_CURSOR);
	}
}

void ColorPicker::orient_progress_slider(size_t control, Cardinal i) const
{
	const UnitRenderable& renderable = ur_wget(*this, control);
	float zero = 0.0f;
	float one = 1.0f;
	renderable.set_attribute_single_vertex(0, 1, i == Cardinal::DOWN || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(1, 1, i == Cardinal::RIGHT || i == Cardinal::DOWN ? &one : &zero);
	renderable.set_attribute_single_vertex(2, 1, i == Cardinal::UP || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(3, 1, i == Cardinal::RIGHT || i == Cardinal::UP ? &one : &zero);
}

void ColorPicker::send_graphic_quad_hue_to_uniform(float hue) const
{
	send_gradient_color_uniform(quad_shader, GradientIndex::GRAPHIC_QUAD, HSV(hue, 1.0f, 1.0f));
}

glm::vec2 ColorPicker::get_graphic_quad_sat_and_value() const
{
	return wp_at(GRAPHIC_QUAD).normalize(wp_at(GRAPHIC_QUAD_CURSOR).transform.position);
}

void ColorPicker::send_graphic_wheel_value_to_uniform(float value) const
{
	Uniforms::send_1(hue_wheel_w_shader, "u_Value", value);
}

void ColorPicker::send_graphic_value_slider_hue_and_sat_to_uniform(float hue, float sat) const
{
	send_gradient_color_uniform(quad_shader, GradientIndex::GRAPHIC_VALUE_SLIDER, HSV(hue, sat, 1.0f));
}

glm::vec2 ColorPicker::get_graphic_wheel_hue_and_sat() const
{
	glm::vec2 normal = 2.0f * wp_at(GRAPHIC_HUE_WHEEL).normalize(wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position) - glm::vec2(1.0f);
	float hue = -glm::atan(normal.y, normal.x) / glm::tau<float>();
	if (hue < 0.0f)
		hue += 1.0f;
	return { hue, glm::length(normal) };
}

void ColorPicker::send_slider_hsv_hue_and_value_to_uniform(float hue, float value) const
{
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ZERO, HSV(hue, 0.0f, value));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ONE, HSV(hue, 1.0f, value));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_V_SLIDER, HSV(hue, 1.0f, 1.0f));
}

void ColorPicker::send_slider_hsl_hue_and_lightness_to_uniform(float hue, float lightness) const
{
	send_gradient_color_uniform(quad_shader, GradientIndex::HSL_S_SLIDER_ZERO, HSL(hue, 0.0f, lightness));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSL_S_SLIDER_ONE, HSL(hue, 1.0f, lightness));
	Uniforms::send_1(linear_lightness_shader, "u_Hue", hue);
}

float ColorPicker::slider_normal_x(size_t control, size_t cursor) const
{
	return wp_at(control).normalize_x(wp_at(cursor).transform.position.x);
}

float ColorPicker::slider_normal_y(size_t control, size_t cursor) const
{
	return wp_at(control).normalize_y(wp_at(cursor).transform.position.y);
}

void ColorPicker::setup_vertex_positions(size_t control) const
{
	const UnitRenderable& renderable = ur_wget(*this, control);
	WidgetPlacement wp = wp_at(control).relative_to(self.transform);
	renderable.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ wp.left(), wp.bottom() }));
	renderable.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ wp.right(), wp.bottom() }));
	renderable.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ wp.left(), wp.top() }));
	renderable.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ wp.right(), wp.top() }));
}

void ColorPicker::setup_rect_uvs(size_t control) const
{
	const UnitRenderable& renderable = ur_wget(*this, control);
	renderable.set_attribute_single_vertex(0, 1, glm::value_ptr(glm::vec2{ 0.0f, 0.0f }));
	renderable.set_attribute_single_vertex(1, 1, glm::value_ptr(glm::vec2{ 1.0f, 0.0f }));
	renderable.set_attribute_single_vertex(2, 1, glm::value_ptr(glm::vec2{ 0.0f, 1.0f }));
	renderable.set_attribute_single_vertex(3, 1, glm::value_ptr(glm::vec2{ 1.0f, 1.0f }));
}

void ColorPicker::setup_gradient(size_t control, GLint g1, GLint g2, GLint g3, GLint g4) const
{
	ur_wget(*this, control).set_attribute(2, glm::value_ptr(glm::vec4{ g1, g2, g3, g4 }));
}

void ColorPicker::sync_cp_widget_with_vp()
{
	sync_single_cp_widget_transform_ur(PREVIEW);
	sync_single_cp_widget_transform_ur(ALPHA_SLIDER);
	sync_single_cp_widget_transform_ur(ALPHA_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(GRAPHIC_QUAD);
	sync_single_cp_widget_transform_ur(GRAPHIC_QUAD_CURSOR);
	sync_single_cp_widget_transform_ur(GRAPHIC_HUE_SLIDER);
	sync_single_cp_widget_transform_ur(GRAPHIC_HUE_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(GRAPHIC_HUE_WHEEL);
	sync_single_cp_widget_transform_ur(GRAPHIC_HUE_WHEEL_CURSOR);
	sync_single_cp_widget_transform_ur(GRAPHIC_VALUE_SLIDER);
	sync_single_cp_widget_transform_ur(GRAPHIC_VALUE_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(RGB_R_SLIDER);
	sync_single_cp_widget_transform_ur(RGB_R_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(RGB_G_SLIDER);
	sync_single_cp_widget_transform_ur(RGB_G_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(RGB_B_SLIDER);
	sync_single_cp_widget_transform_ur(RGB_B_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(HSV_H_SLIDER);
	sync_single_cp_widget_transform_ur(HSV_H_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(HSV_S_SLIDER);
	sync_single_cp_widget_transform_ur(HSV_S_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(HSV_V_SLIDER);
	sync_single_cp_widget_transform_ur(HSV_V_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(HSL_H_SLIDER);
	sync_single_cp_widget_transform_ur(HSL_H_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(HSL_S_SLIDER);
	sync_single_cp_widget_transform_ur(HSL_S_SLIDER_CURSOR);
	sync_single_cp_widget_transform_ur(HSL_L_SLIDER);
	sync_single_cp_widget_transform_ur(HSL_L_SLIDER_CURSOR);
	rr_wget(*this, BACKGROUND).update_transform().send_buffer();
	tr_wget(*this, TEXT_ALPHA).send_vp(*vp);
	tr_wget(*this, TEXT_RED).send_vp(*vp);
	tr_wget(*this, TEXT_GREEN).send_vp(*vp);
	tr_wget(*this, TEXT_BLUE).send_vp(*vp);
	tr_wget(*this, TEXT_HUE).send_vp(*vp);
	tr_wget(*this, TEXT_SAT).send_vp(*vp);
	tr_wget(*this, TEXT_VALUE).send_vp(*vp);
	tr_wget(*this, TEXT_LIGHT).send_vp(*vp);
	b_wget(*this, BUTTON_RGB_HEX_CODE).send_vp();
	b_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).send_vp();
}

void ColorPicker::sync_single_cp_widget_transform_ur(size_t control, bool send_buffer) const
{
	if (children[control])
	{
		setup_vertex_positions(control);
		if (send_buffer)
			ur_wget(*this, control).send_buffer();
	}
}

void ColorPicker::send_cpwc_buffer(size_t control) const
{
	if (children[control])
		ur_wget(*this, control).send_buffer();
}

void ColorPicker::set_circle_cursor_thickness(size_t cursor, float thickness) const
{
	ur_wget(*this, cursor).set_attribute(2, &thickness);
}

void ColorPicker::set_circle_cursor_value(size_t cursor, float value) const
{
	ur_wget(*this, cursor).set_attribute(3, &value);
}

float ColorPicker::get_circle_cursor_value(size_t cursor) const
{
	float value;
	ur_wget(*this, cursor).get_attribute(0, 3, &value);
	return value;
}

void ColorPicker::setup_circle_cursor(size_t cursor)
{
	setup_rect_uvs(cursor);
	set_circle_cursor_thickness(cursor, 0.4f);
	set_circle_cursor_value(cursor, 1.0f);
	wp_at(cursor).transform.scale = { 8, 8 };
}
