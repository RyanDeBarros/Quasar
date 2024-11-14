#include "ColorPicker.h"

#include <glm/gtc/type_ptr.inl>
#include <imgui/imgui_internal.h>

#include "variety/GLutility.h"
#include "edit/color/Color.h"
#include "user/Machine.h"
#include "user/GUI.h"
#include "../render/Uniforms.h"
#include "RoundRect.h"
#include "../text/TextRender.h"
#include "../text/CommonFonts.h"
#include "Button.h"

struct RGBAChangeAction : public ActionBase
{
	ColorPicker& picker;
	RGBA prev_c, new_c;

	RGBAChangeAction(ColorPicker& picker, const RGBA& prev_c, const RGBA& new_c) : picker(picker), prev_c(prev_c), new_c(new_c) { weight = 0.5f; }

	void forward() override
	{
		picker.set_color(new_c, false);
	}

	void backward() override
	{
		picker.set_color(prev_c, false);
	}
};

static const size_t MAIN_TABBAR_BUTTONS[] {
	ColorPicker::BUTTON_GRAPHIC,
	ColorPicker::BUTTON_RGB_SLIDER,
	ColorPicker::BUTTON_HSV_SLIDER,
	ColorPicker::BUTTON_HSL_SLIDER
};

static const size_t ALL_BUTTONS[] {
	ColorPicker::BUTTON_RGB_HEX_CODE,
	ColorPicker::BUTTON_SWITCH_TXTFLD_MODE,
	ColorPicker::BUTTON_QUAD,
	ColorPicker::BUTTON_WHEEL,
	ColorPicker::BUTTON_GRAPHIC,
	ColorPicker::BUTTON_RGB_SLIDER,
	ColorPicker::BUTTON_HSV_SLIDER,
	ColorPicker::BUTTON_HSL_SLIDER
};

// ---------- LAYOUT ----------

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

const float left_text_x = -94;
const float text_sep = 70;
const float text1_y = slider1_y - slider_h * 1.5f;
const float text2_y = text1_y - text_sep;
const float text3_y = text2_y - text_sep;
const float text4_y = text3_y - text_sep;

const float button_switch_txtfld_mode_x = 90;
const float button_rgb_hex_code_w = 45;
const float button_switch_txtfld_mode_w = 22.5f;
const float button_h = 30;

float button_left_x = -115;
float button_top_y_1 = 205;
float button_top_y_2 = 173;
float button_sep_x = 4.25f;
const float button_quad_w = 55;
const float button_wheel_w = 70;
const float button_graphic_w = 80;
const float button_rgb_w = 45;

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
	connect_input_handlers();
	set_color(ColorFrame(), false);
}

ColorPicker::~ColorPicker()
{
	parent_mb_handler.remove_child(&mb_handler);
	parent_key_handler.remove_child(&key_handler);
	Machine.main_window->release_cursor(&wh_interactable);
}

void ColorPicker::draw()
{
	rr_wget(*this, BACKGROUND).draw();
	cp_render_gui_back();
	ur_wget(*this, ALPHA_SLIDER).draw(); // LATER bool member on ColorPicker to enable/disable alpha support
	ur_wget(*this, ALPHA_SLIDER_CURSOR).draw();
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
	ur_wget(*this, PREVIEW).draw();
}

void ColorPicker::process()
{
	process_mb_down_events();
	if (cursor_in_bkg())
	{
		for (size_t button : MAIN_TABBAR_BUTTONS)
			b_t_wget(*this, button).process();
		b_t_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).process();
		if (state & State::SLIDER_RGB)
			b_t_wget(*this, BUTTON_RGB_HEX_CODE).process();
		else if (state & (State::GRAPHIC_QUAD | State::GRAPHIC_WHEEL))
		{
			b_t_wget(*this, BUTTON_QUAD).process();
			b_t_wget(*this, BUTTON_WHEEL).process();
		}
	}
	else
	{
		for (size_t button : MAIN_TABBAR_BUTTONS)
			b_t_wget(*this, button).unhover();
		b_t_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).unhover();
		if (state & State::SLIDER_RGB)
			b_t_wget(*this, BUTTON_RGB_HEX_CODE).unhover();
		else if (state & (State::GRAPHIC_QUAD | State::GRAPHIC_WHEEL))
		{
			b_t_wget(*this, BUTTON_QUAD).unhover();
			b_t_wget(*this, BUTTON_WHEEL).unhover();
		}
	}
}

void ColorPicker::cp_render_gui_back()
{
	ImGui::SetNextWindowBgAlpha(0);
	ImGui::SetNextWindowPos({ gui_transform.position.x, gui_transform.position.y });
	ImGui::SetNextWindowSize({ gui_transform.scale.x, gui_transform.scale.y });

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoBackground;
	if (ImGui::Begin("##color-picker", nullptr, window_flags))
	{
		float font_window_scale = ImGui::GetCurrentWindow()->FontWindowScale;
		ImGui::SetWindowFontScale(scale1d() * font_window_scale);
		popup_hovered = false;
		if (state & State::SLIDER_RGB)
		{
			if (showing_hex_popup)
			{
				Scale psz = { 0.5f * gui_transform.scale.x, 0.18f * gui_transform.scale.y };
				ImGui::SetNextWindowSize({ psz.x, psz.y });
				ImGui::SetNextWindowPos(ImVec2(gui_transform.position.x + 0.5f * gui_transform.scale.x - 0.5f * psz.x,
					gui_transform.position.y + 0.475f * gui_transform.scale.y - 0.5f * psz.y));
				ImGui::OpenPopup("hex-popup", ImGuiPopupFlags_MouseButtonLeft);
			}
			if (ImGui::BeginPopup("hex-popup", ImGuiWindowFlags_NoMove))
			{
				popup_hovered = ImGui::IsWindowHovered();
				if (escape_to_close_popup)
				{
					escape_to_close_popup = false;
					tb_t_wget(*this, BUTTON_RGB_HEX_CODE).deselect();
				}
				if (showing_hex_popup)
				{
					ImGui::Text("RGB hex code");
					ImGui::Text("#");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(100 * self.transform.scale.x);
					if (ImGui::InputText("##hex-popup-txtfld", rgb_hex, rgb_hex_size))
						update_rgb_hex();
				}
				else
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}

		float alpha = get_color().alpha;
		const float imgui_slider_w = 200;
		const float imgui_y_1 = 145 * self.transform.scale.y;
		const float imgui_y_2 = imgui_y_1 + slider_sep * Machine.get_app_scale().y * self.transform.scale.y;
		const float imgui_y_3 = imgui_y_2 + slider_sep * Machine.get_app_scale().y * self.transform.scale.y;
		const float imgui_y_4 = imgui_y_3 + slider_sep * Machine.get_app_scale().y * self.transform.scale.y;
		const float imgui_sml_x = 120 * self.transform.scale.x;
		if (state & State::SLIDER_RGB)
		{
			RGB rgb = get_color().rgb();
			if (txtfld_mode == TextFieldMode::NUMBER)
			{
				bool mod = false;
				// LATER should the get_pixel_ functions be doing static_cast<int> instead of roundi? for instance, 0.5f currently becomes 128, not 127.
				// Make sure though that in static_cast<int> case, 1.0 becomes 255 and not 254.
				int r = rgb.get_pixel_r(), g = rgb.get_pixel_g(), b = rgb.get_pixel_b();
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-red", &r, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-green", &g, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-blue", &b, 5, 10);
				if (mod)
					set_color(ColorFrame(RGB(r, g, b), alpha), true);
			}
			else if (txtfld_mode == TextFieldMode::PERCENT)
			{
				bool mod = false;
				float r = rgb.r * 100, g = rgb.g * 100, b = rgb.b * 100;
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-red", &r, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-green", &g, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-blue", &b, 5, 10, "%.2f");
				if (mod)
					set_color(RGBA(r * 0.01f, g * 0.01f, b * 0.01f, alpha), true);
			}
		}
		else if (state & State::SLIDER_HSV)
		{
			HSV hsv = get_color().hsv();
			if (txtfld_mode == TextFieldMode::NUMBER)
			{
				bool mod = false;
				int h = hsv.get_pixel_h(), s = hsv.get_pixel_s(), v = hsv.get_pixel_v();
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-hue", &h, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-sat", &s, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-value", &v, 5, 10);
				if (mod)
					set_color(ColorFrame(HSV(h, s, v), alpha), true);
			}
			else if (txtfld_mode == TextFieldMode::PERCENT)
			{
				bool mod = false;
				float h = hsv.h * 100, s = hsv.s * 100, v = hsv.v * 100;
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-hue", &h, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-sat", &s, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-value", &v, 5, 10, "%.2f");
				if (mod)
					set_color(HSVA(h * 0.01f, s * 0.01f, v * 0.01f, alpha), true);
			}
		}
		else if (state & State::SLIDER_HSL)
		{
			HSL hsl = get_color().hsl();
			if (txtfld_mode == TextFieldMode::NUMBER)
			{
				bool mod = false;
				int h = hsl.get_pixel_h(), s = hsl.get_pixel_s(), l = hsl.get_pixel_l();
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-hue", &h, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-sat", &s, 5, 10);
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputInt("##it-light", &l, 5, 10);
				if (mod)
					set_color(ColorFrame(HSL(h, s, l), alpha), true);
			}
			else if (txtfld_mode == TextFieldMode::PERCENT)
			{
				bool mod = false;
				float h = hsl.h * 100, s = hsl.s * 100, l = hsl.l * 100;
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_1));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-hue", &h, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_2));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-sat", &s, 5, 10, "%.2f");
				ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_3));
				ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
				mod |= ImGui::InputFloat("##it-light", &l, 5, 10, "%.2f");
				if (mod)
					set_color(HSLA(h * 0.01f, s * 0.01f, l * 0.01f, alpha), true);
			}
		}
		if (txtfld_mode == TextFieldMode::NUMBER)
		{
			ColorFrame color = get_color();
			int a = color.get_pixel_a();
			ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_4)); 
			ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
			if (ImGui::InputInt("##it-alpha", &a, 5, 10))
			{
				color.set_pixel_a(a);
				set_color(color, true);
			}
		}
		else if (txtfld_mode == TextFieldMode::PERCENT)
		{
			ColorFrame color = get_color();
			float a = color.alpha * 100;
			ImGui::SetCursorPos(ImVec2(imgui_sml_x, imgui_y_4));
			ImGui::SetNextItemWidth(imgui_slider_w * self.transform.scale.x);
			if (ImGui::InputFloat("##it-alpha", &a, 5, 10, "%.2f"))
				set_color(ColorFrame(color.rgb(), a * 0.01f), true);
		}

		ImGui::SetWindowFontScale(font_window_scale);
		ImGui::End();
	}
}

void ColorPicker::cp_render_gui_front()
{
	b_t_wget(*this, BUTTON_SWITCH_TXTFLD_MODE).draw();
	if (state & (State::GRAPHIC_QUAD | State::GRAPHIC_WHEEL))
		sub_tab_bar.draw();
	else if (state & State::SLIDER_RGB)
	{
		if (showing_hex_popup)
			rr_wget(*this, BACKGROUND).draw();
		b_t_wget(*this, BUTTON_RGB_HEX_CODE).draw();
		tr_wget(*this, TEXT_RED).draw();
		tr_wget(*this, TEXT_GREEN).draw();
		tr_wget(*this, TEXT_BLUE).draw();
	}
	else if (state & State::SLIDER_HSV)
	{
		tr_wget(*this, TEXT_HUE).draw();
		tr_wget(*this, TEXT_SAT).draw();
		tr_wget(*this, TEXT_VALUE).draw();
	}
	else if (state & State::SLIDER_HSL)
	{
		tr_wget(*this, TEXT_HUE).draw();
		tr_wget(*this, TEXT_SAT).draw();
		tr_wget(*this, TEXT_LIGHT).draw();
	}
	tr_wget(*this, TEXT_ALPHA).draw();
	main_tab_bar.draw();
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
	set_color(RGBA(RGB(hex), slider_normal_x(ALPHA_SLIDER, ALPHA_SLIDER_CURSOR)), true);
}

void ColorPicker::set_state(State _state)
{
	if (_state != state)
	{
		if (_state & State::GRAPHIC_QUAD)
			last_graphic_state = State::GRAPHIC_QUAD;
		else if (_state & State::GRAPHIC_WHEEL)
			last_graphic_state = State::GRAPHIC_WHEEL;
		else if (state & (State::GRAPHIC_QUAD | State::GRAPHIC_WHEEL))
			last_graphic_state = state;

		release_cursor();
		ColorFrame pre_color = get_color();
		state = _state;
		set_color(pre_color, false);

		b_t_wget(*this, BUTTON_RGB_HEX_CODE).enabled = state & State::SLIDER_RGB;
		b_t_wget(*this, BUTTON_QUAD).enabled = b_t_wget(*this, BUTTON_WHEEL).enabled = state & (State::GRAPHIC_QUAD | State::GRAPHIC_WHEEL);

		if (state & State::GRAPHIC_QUAD)
		{
			main_tab_bar.select(BUTTON_GRAPHIC);
			sub_tab_bar.select(BUTTON_QUAD);
		}
		else if (state & State::GRAPHIC_WHEEL)
		{
			main_tab_bar.select(BUTTON_GRAPHIC);
			sub_tab_bar.select(BUTTON_WHEEL);
		}
		else if (state & State::SLIDER_RGB)
			main_tab_bar.select(BUTTON_RGB_SLIDER);
		else if (state & State::SLIDER_HSV)
			main_tab_bar.select(BUTTON_HSV_SLIDER);
		else if (state & State::SLIDER_HSL)
			main_tab_bar.select(BUTTON_HSL_SLIDER);
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
	sync_widget_with_vp();
}

void ColorPicker::initialize_widget()
{
	// ---------- GRAPHIC QUAD ----------

	assign_widget(this, GRAPHIC_QUAD, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, GRAPHIC_QUAD_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));
	assign_widget(this, GRAPHIC_HUE_SLIDER, std::make_shared<W_UnitRenderable>(&linear_hue_shader));
	assign_widget(this, GRAPHIC_HUE_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));

	setup_rect_uvs(GRAPHIC_QUAD);
	setup_gradient(GRAPHIC_QUAD, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::BLACK,
		(GLint)GradientIndex::WHITE, (GLint)GradientIndex::GRAPHIC_QUAD);
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

	assign_widget(this, GRAPHIC_HUE_WHEEL, std::make_shared<W_UnitRenderable>(&hue_wheel_w_shader));
	assign_widget(this, GRAPHIC_HUE_WHEEL_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));
	assign_widget(this, GRAPHIC_VALUE_SLIDER, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, GRAPHIC_VALUE_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));

	setup_rect_uvs(GRAPHIC_HUE_WHEEL);
	send_graphic_wheel_value_to_uniform(1.0f);
	setup_circle_cursor(GRAPHIC_HUE_WHEEL_CURSOR);
	setup_rect_uvs(GRAPHIC_VALUE_SLIDER);
	setup_gradient(GRAPHIC_VALUE_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::BLACK,
		(GLint)GradientIndex::GRAPHIC_VALUE_SLIDER, (GLint)GradientIndex::GRAPHIC_VALUE_SLIDER);
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

	assign_widget(this, RGB_R_SLIDER, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, RGB_R_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));
	assign_widget(this, RGB_G_SLIDER, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, RGB_G_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));
	assign_widget(this, RGB_B_SLIDER, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, RGB_B_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));

	setup_rect_uvs(RGB_R_SLIDER);
	setup_circle_cursor(RGB_R_SLIDER_CURSOR);
	setup_gradient(RGB_R_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_R_SLIDER,
		(GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_R_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::RGB_R_SLIDER, RGB(0xFF0000));
	setup_rect_uvs(RGB_G_SLIDER);
	setup_circle_cursor(RGB_G_SLIDER_CURSOR);
	setup_gradient(RGB_G_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_G_SLIDER,
		(GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_G_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::RGB_G_SLIDER, RGB(0x00FF00));
	setup_rect_uvs(RGB_B_SLIDER);
	setup_circle_cursor(RGB_B_SLIDER_CURSOR);
	setup_gradient(RGB_B_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_B_SLIDER,
		(GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_B_SLIDER);
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

	assign_widget(this, HSV_H_SLIDER, std::make_shared<W_UnitRenderable>(&linear_hue_shader));
	assign_widget(this, HSV_H_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));
	assign_widget(this, HSV_S_SLIDER, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, HSV_S_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));
	assign_widget(this, HSV_V_SLIDER, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, HSV_V_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));

	orient_progress_slider(HSV_H_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(HSV_H_SLIDER_CURSOR);
	setup_rect_uvs(HSV_S_SLIDER);
	setup_circle_cursor(HSV_S_SLIDER_CURSOR);
	setup_gradient(HSV_S_SLIDER, (GLint)GradientIndex::HSV_S_SLIDER_ZERO, (GLint)GradientIndex::HSV_S_SLIDER_ONE,
		(GLint)GradientIndex::HSV_S_SLIDER_ZERO, (GLint)GradientIndex::HSV_S_SLIDER_ONE);
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ZERO, HSV(0.0f, 0.0f, 1.0f));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ONE, HSV(0.0f, 1.0f, 1.0f));
	setup_rect_uvs(HSV_V_SLIDER);
	setup_circle_cursor(HSV_V_SLIDER_CURSOR);
	setup_gradient(HSV_V_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::HSV_V_SLIDER,
		(GLint)GradientIndex::BLACK, (GLint)GradientIndex::HSV_V_SLIDER);
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

	assign_widget(this, HSL_H_SLIDER, std::make_shared<W_UnitRenderable>(&linear_hue_shader));
	assign_widget(this, HSL_H_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));
	assign_widget(this, HSL_S_SLIDER, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, HSL_S_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));
	assign_widget(this, HSL_L_SLIDER, std::make_shared<W_UnitRenderable>(&linear_lightness_shader));
	assign_widget(this, HSL_L_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));

	orient_progress_slider(HSL_H_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(HSL_H_SLIDER_CURSOR);
	setup_rect_uvs(HSL_S_SLIDER);
	setup_circle_cursor(HSL_S_SLIDER_CURSOR);
	setup_gradient(HSL_S_SLIDER, (GLint)GradientIndex::HSL_S_SLIDER_ZERO, (GLint)GradientIndex::HSL_S_SLIDER_ONE,
		(GLint)GradientIndex::HSL_S_SLIDER_ZERO, (GLint)GradientIndex::HSL_S_SLIDER_ONE);
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
	
	assign_widget(this, ALPHA_SLIDER, std::make_shared<W_UnitRenderable>(&quad_shader));
	assign_widget(this, ALPHA_SLIDER_CURSOR, std::make_shared<W_UnitRenderable>(&circle_cursor_shader));

	setup_rect_uvs(ALPHA_SLIDER);
	setup_circle_cursor(ALPHA_SLIDER_CURSOR);
	setup_gradient(ALPHA_SLIDER, (GLint)GradientIndex::TRANSPARENT, (GLint)GradientIndex::ALPHA_SLIDER,
		(GLint)GradientIndex::TRANSPARENT, (GLint)GradientIndex::ALPHA_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::ALPHA_SLIDER, ColorFrame());

	wp_at(ALPHA_SLIDER).transform.position.x = slider_x;
	wp_at(ALPHA_SLIDER).transform.position.y = slider4_y;
	wp_at(ALPHA_SLIDER).transform.scale = { slider_w, slider_h };
	wp_at(ALPHA_SLIDER_CURSOR).transform.position = { wp_at(ALPHA_SLIDER).right(), wp_at(ALPHA_SLIDER).center_y() };

	// ---------- PREVIEW ----------
	
	assign_widget(this, PREVIEW, std::make_shared<W_UnitRenderable>(&quad_shader));
	setup_rect_uvs(PREVIEW);
	setup_gradient(PREVIEW, (GLint)GradientIndex::PREVIEW, (GLint)GradientIndex::PREVIEW,
		(GLint)GradientIndex::PREVIEW, (GLint)GradientIndex::PREVIEW);
	send_gradient_color_uniform(quad_shader, GradientIndex::PREVIEW, ColorFrame());
	wp_at(PREVIEW).transform.position.x = preview_x;
	wp_at(PREVIEW).transform.position.y = preview_y;
	wp_at(PREVIEW).transform.scale = { preview_w, preview_h };
	wp_at(PREVIEW).pivot.y = 1;

	// ---------- BACKGROUND ----------

	assign_widget(this, BACKGROUND, std::make_shared<RoundRect>(&round_rect_shader));
	rr_wget(*this, BACKGROUND).thickness = 0.25f;
	rr_wget(*this, BACKGROUND).corner_radius = 10;
	rr_wget(*this, BACKGROUND).border_color = RGBA(HSV(0.7f, 0.5f, 0.5f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).update_all();

	// ---------- TEXT ----------

	assign_widget(this, TEXT_ALPHA, std::make_shared<TextRender>(*Fonts::label_regular, 18, "Alpha"));
	wp_at(TEXT_ALPHA).pivot.y = 0.5f;
	tr_wget(*this, TEXT_ALPHA).setup_renderable();
	wp_at(TEXT_ALPHA).transform.position.x = left_text_x;
	wp_at(TEXT_ALPHA).transform.position.y = text4_y;

	assign_widget(this, TEXT_RED, std::make_shared<TextRender>(*Fonts::label_regular, 18, "Red"));
	wp_at(TEXT_RED).pivot.y = 0.5f;
	tr_wget(*this, TEXT_RED).setup_renderable();
	wp_at(TEXT_RED).transform.position.x = left_text_x;
	wp_at(TEXT_RED).transform.position.y = text1_y;

	assign_widget(this, TEXT_GREEN, std::make_shared<TextRender>(*Fonts::label_regular, 18, "Green"));
	wp_at(TEXT_GREEN).pivot.y = 0.5f;
	tr_wget(*this, TEXT_GREEN).setup_renderable();
	wp_at(TEXT_GREEN).transform.position.x = left_text_x;
	wp_at(TEXT_GREEN).transform.position.y = text2_y;

	assign_widget(this, TEXT_BLUE, std::make_shared<TextRender>(*Fonts::label_regular, 18, "Blue"));
	wp_at(TEXT_BLUE).pivot.y = 0.5f;
	tr_wget(*this, TEXT_BLUE).setup_renderable();
	wp_at(TEXT_BLUE).transform.position.x = left_text_x;
	wp_at(TEXT_BLUE).transform.position.y = text3_y;

	assign_widget(this, TEXT_HUE, std::make_shared<TextRender>(*Fonts::label_regular, 18, "Hue"));
	wp_at(TEXT_HUE).pivot.y = 0.5f;
	tr_wget(*this, TEXT_HUE).setup_renderable();
	wp_at(TEXT_HUE).transform.position.x = left_text_x;
	wp_at(TEXT_HUE).transform.position.y = text1_y;

	assign_widget(this, TEXT_SAT, std::make_shared<TextRender>(*Fonts::label_regular, 18, "Sat"));
	wp_at(TEXT_SAT).pivot.y = 0.5f;
	tr_wget(*this, TEXT_SAT).setup_renderable();
	wp_at(TEXT_SAT).transform.position.x = left_text_x;
	wp_at(TEXT_SAT).transform.position.y = text2_y;

	assign_widget(this, TEXT_VALUE, std::make_shared<TextRender>(*Fonts::label_regular, 18, "Value"));
	wp_at(TEXT_VALUE).pivot.y = 0.5f;
	tr_wget(*this, TEXT_VALUE).setup_renderable();
	wp_at(TEXT_VALUE).transform.position.x = left_text_x;
	wp_at(TEXT_VALUE).transform.position.y = text3_y;

	assign_widget(this, TEXT_LIGHT, std::make_shared<TextRender>(*Fonts::label_regular, 18, "Light"));
	wp_at(TEXT_LIGHT).pivot.y = 0.5f;
	tr_wget(*this, TEXT_LIGHT).setup_renderable();
	wp_at(TEXT_LIGHT).transform.position.x = left_text_x;
	wp_at(TEXT_LIGHT).transform.position.y = text3_y;
	
	// ---------- BUTTONS ----------

	StandardTButtonArgs sba(&mb_handler, &round_rect_shader, vp);
	sba.pivot = { 0, 1 };

	sba.text = "#";
	sba.transform = { { button_switch_txtfld_mode_x, button_top_y_2 }, { button_switch_txtfld_mode_w, button_h } };
	sba.is_hoverable = [this]() { return current_widget_control == -1; };
	assign_widget(this, BUTTON_SWITCH_TXTFLD_MODE, std::make_shared<StandardTButton>(sba));
	StandardTButton& b_txtfld = sb_t_wget(*this, BUTTON_SWITCH_TXTFLD_MODE);
	b_txtfld.on_select = fconv_st_on_action([this, &b_txtfld]() {
		if (txtfld_mode == TextFieldMode::NUMBER)
		{
			txtfld_mode = TextFieldMode::PERCENT;
			b_txtfld.text().set_text("%");
		}
		else if (txtfld_mode == TextFieldMode::PERCENT)
		{
			txtfld_mode = TextFieldMode::NUMBER;
			b_txtfld.text().set_text("#");
		}
		});

	ToggleTButtonArgs tba(&mb_handler, &round_rect_shader, vp);
	tba.pivot = { 0, 1 };

	tba.text = "HEX";
	tba.transform = { { button_left_x, button_top_y_2 }, { button_rgb_hex_code_w, button_h } };
	auto tb = std::make_shared<ToggleTButton>(tba);
	tb->is_hoverable = [this]() { return state & State::SLIDER_RGB && current_widget_control == -1; };
	tb->on_select = fconv_tt_on_action([this]() { showing_hex_popup = true; });
	tb->on_deselect = fconv_tt_on_action([this]() { showing_hex_popup = false; });
	assign_widget(this, BUTTON_RGB_HEX_CODE, tb);

	tba.is_hoverable = [this]() { return current_widget_control == -1; };
	tba.text = "QUAD";
	tba.transform.scale = { button_quad_w, button_h };
	tba.is_hoverable = [this]() { return state & (State::GRAPHIC_QUAD | State::GRAPHIC_WHEEL) && current_widget_control == -1; };
	tba.on_select = fconv_tt_on_action([this]() { set_state(State::GRAPHIC_QUAD); });
	assign_widget(this, BUTTON_QUAD, std::make_shared<ToggleTButton>(tba));
	
	tba.text = "WHEEL";
	tba.transform.position.x += tba.transform.scale.x + button_sep_x;
	tba.transform.scale.x = button_wheel_w;
	tba.on_select = fconv_tt_on_action([this]() { set_state(State::GRAPHIC_WHEEL); });
	assign_widget(this, BUTTON_WHEEL, std::make_shared<ToggleTButton>(tba));

	sub_tab_bar.init({
		{ BUTTON_QUAD, &tb_t_wget(*this, BUTTON_QUAD) },
		{ BUTTON_WHEEL, &tb_t_wget(*this, BUTTON_WHEEL) },
		}, BUTTON_QUAD);

	tba.text = "GRAPHIC";
	tba.transform.position = { button_left_x, button_top_y_1 };
	tba.transform.scale.x = button_graphic_w;
	tba.is_hoverable = [this]() { return current_widget_control == -1; };
	tba.on_select = fconv_tt_on_action([this]() { set_state(last_graphic_state); });
	assign_widget(this, BUTTON_GRAPHIC, std::make_shared<ToggleTButton>(tba));

	tba.text = "RGB";
	tba.transform.position.x += tba.transform.scale.x + button_sep_x;
	tba.transform.scale.x = button_rgb_w;
	tba.on_select = fconv_tt_on_action([this]() { set_state(State::SLIDER_RGB); });
	assign_widget(this, BUTTON_RGB_SLIDER, std::make_shared<ToggleTButton>(tba));

	tba.text = "HSV";
	tba.transform.position.x += tba.transform.scale.x + button_sep_x;
	tba.on_select = fconv_tt_on_action([this]() { set_state(State::SLIDER_HSV); });
	assign_widget(this, BUTTON_HSV_SLIDER, std::make_shared<ToggleTButton>(tba));

	tba.text = "HSL";
	tba.transform.position.x += tba.transform.scale.x + button_sep_x;
	tba.on_select = fconv_tt_on_action([this]() { set_state(State::SLIDER_HSL); });
	assign_widget(this, BUTTON_HSL_SLIDER, std::make_shared<ToggleTButton>(tba));

	main_tab_bar.init({
		{ BUTTON_GRAPHIC, &tb_t_wget(*this, BUTTON_GRAPHIC) },
		{ BUTTON_RGB_SLIDER, &tb_t_wget(*this, BUTTON_RGB_SLIDER) },
		{ BUTTON_HSV_SLIDER, &tb_t_wget(*this, BUTTON_HSV_SLIDER) },
		{ BUTTON_HSL_SLIDER, &tb_t_wget(*this, BUTTON_HSL_SLIDER) }
		}, BUTTON_GRAPHIC);
}

void ColorPicker::connect_input_handlers()
{
	parent_mb_handler.children.push_back(&mb_handler);
	mb_handler.callback = [this](const MouseButtonEvent& mb) {
		if (showing_hex_popup && cursor_in_bkg())
		{
			mb.consumed = true;
			if (mb.action == IAction::PRESS && showing_hex_popup && !popup_hovered)
				tb_t_wget(*this, BUTTON_RGB_HEX_CODE).deselect();
			return;
		}
		if (mb.button != MouseButton::LEFT)
			return;
		if (mb.action == IAction::PRESS && cursor_in_bkg())
		{
			if (current_widget_control < 0)
			{
				// TODO simplify these IF statements
				Position local_cursor_pos = local_of(Machine.cursor_world_pos(glm::inverse(*vp)));
				if (wp_at(ALPHA_SLIDER).contains_point(local_cursor_pos))
				{
					take_over_cursor();
					current_widget_control = ALPHA_SLIDER_CURSOR;
					mb.consumed = true;
					current_action_color = get_color().rgba();
					mouse_handler_horizontal_slider(ALPHA_SLIDER, ALPHA_SLIDER_CURSOR, local_cursor_pos);
				}
				else if (state & State::GRAPHIC_QUAD)
				{
					if (wp_at(GRAPHIC_QUAD).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_QUAD_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_graphic_quad(local_cursor_pos);
					}
					else if (wp_at(GRAPHIC_HUE_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_HUE_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_vertical_slider(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR, local_cursor_pos);
					}
				}
				else if (state & State::GRAPHIC_WHEEL)
				{
					if (wp_at(GRAPHIC_HUE_WHEEL).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_HUE_WHEEL_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_graphic_hue_wheel(local_cursor_pos);
					}
					else if (wp_at(GRAPHIC_VALUE_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_VALUE_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_vertical_slider(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR, local_cursor_pos);
					}
				}
				else if (state & State::SLIDER_RGB)
				{
					if (wp_at(RGB_R_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_R_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(RGB_R_SLIDER, RGB_R_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(RGB_G_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_G_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(RGB_G_SLIDER, RGB_G_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(RGB_B_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_B_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(RGB_B_SLIDER, RGB_B_SLIDER_CURSOR, local_cursor_pos);
					}
				}
				else if (state & State::SLIDER_HSV)
				{
					if (wp_at(HSV_H_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_H_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(HSV_S_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_S_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(HSV_V_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_V_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR, local_cursor_pos);
					}
				}
				else if (state & State::SLIDER_HSL)
				{
					if (wp_at(HSL_H_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_H_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(HSL_S_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_S_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR, local_cursor_pos);
					}
					else if (wp_at(HSL_L_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_L_SLIDER_CURSOR;
						mb.consumed = true;
						current_action_color = get_color().rgba();
						mouse_handler_horizontal_slider(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR, local_cursor_pos);
					}
				}

				if (current_widget_control >= 0)
					update_display_colors();
			}
		}
		else if (mb.action == IAction::RELEASE)
		{
			if (current_widget_control >= 0)
			{
				mb.consumed = true;
				release_cursor();

				auto action = std::make_shared<RGBAChangeAction>(*this, current_action_color, get_color().rgba());
				Machine.history.push(std::move(action));
			}
		}
	};

	parent_key_handler.children.push_back(&key_handler);
	key_handler.callback = [this](const KeyEvent& k) {
		if (ImGui::GetIO().WantCaptureKeyboard)
			k.consumed = true;
		else if (k.action == IAction::PRESS && k.key == Key::ESCAPE && showing_hex_popup && cursor_in_bkg())
		{
			k.consumed = true;
			escape_to_close_popup = true;
		}
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

void ColorPicker::take_over_cursor()
{
	Machine.main_window->request_cursor(&wh_interactable, StandardCursor::CROSSHAIR);
}

void ColorPicker::release_cursor()
{
	if (current_widget_control >= 0)
	{
		Machine.main_window->release_cursor(&wh_interactable);
		current_widget_control = -1;
	}
}

ColorFrame ColorPicker::get_color() const
{
	ColorFrame color(slider_normal_x(ALPHA_SLIDER, ALPHA_SLIDER_CURSOR));
	if (state & State::GRAPHIC_QUAD)
	{
		glm::vec2 sv = get_graphic_quad_sat_and_value();
		color.set_hsv(HSV(slider_normal_y(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR), sv[0], sv[1]));
	}
	else if (state & State::GRAPHIC_WHEEL)
	{
		glm::vec2 hs = get_graphic_wheel_hue_and_sat();
		color.set_hsv(HSV(hs[0], hs[1], slider_normal_y(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR)));
	}
	else if (state & State::SLIDER_RGB)
	{
		float r = slider_normal_x(RGB_R_SLIDER, RGB_R_SLIDER_CURSOR);
		float g = slider_normal_x(RGB_G_SLIDER, RGB_G_SLIDER_CURSOR);
		float b = slider_normal_x(RGB_B_SLIDER, RGB_B_SLIDER_CURSOR);
		color.set_rgb(RGB(r, g, b));
	}
	else if (state & State::SLIDER_HSV)
	{
		float h = slider_normal_x(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR);
		float s = slider_normal_x(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR);
		float v = slider_normal_x(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR);
		color.set_hsv(HSV(h, s, v));
	}
	else if (state & State::SLIDER_HSL)
	{
		float h = slider_normal_x(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR);
		float s = slider_normal_x(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR);
		float l = slider_normal_x(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR);
		color.set_hsl(HSL(h, s, l));
	}
	return color;
}

void ColorPicker::set_color(ColorFrame color, bool create_action)
{
	ColorFrame prev_color = get_color();
	move_slider_cursor_x_relative(ALPHA_SLIDER, ALPHA_SLIDER_CURSOR, color.alpha);
	sync_single_standard_ur_transform(ALPHA_SLIDER_CURSOR);
	if (state & State::GRAPHIC_QUAD)
	{
		HSV hsv = color.hsv();
		wp_at(GRAPHIC_QUAD_CURSOR).transform.position.x = wp_at(GRAPHIC_QUAD).interp_x(hsv.s);
		wp_at(GRAPHIC_QUAD_CURSOR).transform.position.y = wp_at(GRAPHIC_QUAD).interp_y(hsv.v);
		move_slider_cursor_y_relative(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR, hsv.h);
		sync_single_standard_ur_transform(GRAPHIC_QUAD_CURSOR);
		sync_single_standard_ur_transform(GRAPHIC_HUE_SLIDER_CURSOR);
	}
	else if (state & State::GRAPHIC_WHEEL)
	{
		HSV hsv = color.hsv();
		float x = hsv.s * glm::cos(glm::tau<float>() * hsv.h);
		float y = -hsv.s * glm::sin(glm::tau<float>() * hsv.h);
		wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position.x = wp_at(GRAPHIC_HUE_WHEEL).interp_x(0.5f * (x + 1));
		wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position.y = wp_at(GRAPHIC_HUE_WHEEL).interp_y(0.5f * (y + 1));
		move_slider_cursor_y_relative(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR, hsv.v);
		sync_single_standard_ur_transform(GRAPHIC_HUE_WHEEL_CURSOR);
		sync_single_standard_ur_transform(GRAPHIC_VALUE_SLIDER_CURSOR);
	}
	else if (state & State::SLIDER_RGB)
	{
		RGB rgb = color.rgb();
		move_slider_cursor_x_relative(RGB_R_SLIDER, RGB_R_SLIDER_CURSOR, rgb.r);
		move_slider_cursor_x_relative(RGB_G_SLIDER, RGB_G_SLIDER_CURSOR, rgb.g);
		move_slider_cursor_x_relative(RGB_B_SLIDER, RGB_B_SLIDER_CURSOR, rgb.b);
		sync_single_standard_ur_transform(RGB_R_SLIDER_CURSOR);
		sync_single_standard_ur_transform(RGB_G_SLIDER_CURSOR);
		sync_single_standard_ur_transform(RGB_B_SLIDER_CURSOR);
	}
	else if (state & State::SLIDER_HSV)
	{
		HSV hsv = color.hsv();
		move_slider_cursor_x_relative(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR, hsv.h);
		move_slider_cursor_x_relative(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR, hsv.s);
		move_slider_cursor_x_relative(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR, hsv.v);
		sync_single_standard_ur_transform(HSV_H_SLIDER_CURSOR);
		sync_single_standard_ur_transform(HSV_S_SLIDER_CURSOR);
		sync_single_standard_ur_transform(HSV_V_SLIDER_CURSOR);
	}
	else if (state & State::SLIDER_HSL)
	{
		HSL hsl = color.hsl();
		move_slider_cursor_x_relative(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR, hsl.h);
		move_slider_cursor_x_relative(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR, hsl.s);
		move_slider_cursor_x_relative(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR, hsl.l);
		sync_single_standard_ur_transform(HSL_H_SLIDER_CURSOR);
		sync_single_standard_ur_transform(HSL_S_SLIDER_CURSOR);
		sync_single_standard_ur_transform(HSL_L_SLIDER_CURSOR);
	}
	update_display_colors();

	if (create_action && color != prev_color)
	{
		auto action = std::make_shared<RGBAChangeAction>(*this, prev_color.rgba(), color.rgba());
		Machine.history.push(std::move(action));
	}
}

void ColorPicker::set_size(Scale size, bool sync)
{
	wp_at(BACKGROUND).transform.scale = size;
	rr_wget(*this, BACKGROUND).update_transform();
	if (sync)
		sync_widget_with_vp();
}

Scale ColorPicker::minimum_display() const
{
	return Scale{ 240, 420 };
}

void ColorPicker::mouse_handler_graphic_quad(Position local_cursor_pos)
{
	wp_at(GRAPHIC_QUAD_CURSOR).transform.position = wp_at(GRAPHIC_QUAD).clamp_point(local_cursor_pos);
	sync_single_standard_ur_transform(GRAPHIC_QUAD_CURSOR);
}

void ColorPicker::mouse_handler_graphic_hue_wheel(Position local_cursor_pos)
{
	wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position = wp_at(GRAPHIC_HUE_WHEEL).clamp_point_in_ellipse(local_cursor_pos);
	sync_single_standard_ur_transform(GRAPHIC_HUE_WHEEL_CURSOR);
}

void ColorPicker::mouse_handler_horizontal_slider(size_t slider, size_t cursor, Position local_cursor_pos)
{
	move_slider_cursor_x_absolute(slider, cursor, local_cursor_pos.x);
	sync_single_standard_ur_transform(cursor);
}

void ColorPicker::mouse_handler_vertical_slider(size_t slider, size_t cursor, Position local_cursor_pos)
{
	move_slider_cursor_y_absolute(slider, cursor, local_cursor_pos.y);
	sync_single_standard_ur_transform(cursor);
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
	if (state & State::GRAPHIC_QUAD)
	{
		HSV hsv = color.hsv();
		set_circle_cursor_value(GRAPHIC_QUAD_CURSOR, contrast_wb_value_complex_hsv(hsv));
		send_cpwc_buffer(GRAPHIC_QUAD_CURSOR);
		send_graphic_quad_hue_to_uniform(hsv.h);
		set_circle_cursor_value(GRAPHIC_HUE_SLIDER_CURSOR, contrast_wb_value_simple_hue(hsv.h));
		send_cpwc_buffer(GRAPHIC_HUE_SLIDER_CURSOR);
	}
	else if (state & State::GRAPHIC_WHEEL)
	{
		HSV hsv = color.hsv();
		set_circle_cursor_value(GRAPHIC_HUE_WHEEL_CURSOR, contrast_wb_value_simple_hue_and_sat(hsv.h, hsv.s));
		send_cpwc_buffer(GRAPHIC_HUE_WHEEL_CURSOR);
		send_graphic_value_slider_hue_and_sat_to_uniform(hsv.h, hsv.s);
		// LATER better contrast functions
		set_circle_cursor_value(GRAPHIC_VALUE_SLIDER_CURSOR, contrast_wb_value_simple_hue_and_value(hsv.h, hsv.v));
		send_graphic_wheel_value_to_uniform(hsv.v);
		send_cpwc_buffer(GRAPHIC_VALUE_SLIDER_CURSOR);
	}
	else if (state & State::SLIDER_RGB)
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
	else if (state & State::SLIDER_HSV)
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
	else if (state & State::SLIDER_HSL)
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
	const UnitRenderable& ur = ur_wget(*this, control);
	WidgetPlacement wp = wp_at(control).relative_to(self.transform);
	ur.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ wp.left(), wp.bottom() }));
	ur.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ wp.right(), wp.bottom() }));
	ur.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ wp.left(), wp.top() }));
	ur.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ wp.right(), wp.top() }));
}

void ColorPicker::setup_rect_uvs(size_t control) const
{
	const UnitRenderable& ur = ur_wget(*this, control);
	ur.set_attribute_single_vertex(0, 1, glm::value_ptr(glm::vec2{ 0.0f, 0.0f }));
	ur.set_attribute_single_vertex(1, 1, glm::value_ptr(glm::vec2{ 1.0f, 0.0f }));
	ur.set_attribute_single_vertex(2, 1, glm::value_ptr(glm::vec2{ 0.0f, 1.0f }));
	ur.set_attribute_single_vertex(3, 1, glm::value_ptr(glm::vec2{ 1.0f, 1.0f }));
}

void ColorPicker::setup_gradient(size_t control, GLint g1, GLint g2, GLint g3, GLint g4) const
{
	ur_wget(*this, control).set_attribute(2, glm::value_ptr(glm::vec4{ g1, g2, g3, g4 }));
}

void ColorPicker::sync_widget_with_vp()
{
	sync_single_standard_ur_transform(PREVIEW);
	sync_single_standard_ur_transform(ALPHA_SLIDER);
	sync_single_standard_ur_transform(ALPHA_SLIDER_CURSOR);
	sync_single_standard_ur_transform(GRAPHIC_QUAD);
	sync_single_standard_ur_transform(GRAPHIC_QUAD_CURSOR);
	sync_single_standard_ur_transform(GRAPHIC_HUE_SLIDER);
	sync_single_standard_ur_transform(GRAPHIC_HUE_SLIDER_CURSOR);
	sync_single_standard_ur_transform(GRAPHIC_HUE_WHEEL);
	sync_single_standard_ur_transform(GRAPHIC_HUE_WHEEL_CURSOR);
	sync_single_standard_ur_transform(GRAPHIC_VALUE_SLIDER);
	sync_single_standard_ur_transform(GRAPHIC_VALUE_SLIDER_CURSOR);
	sync_single_standard_ur_transform(RGB_R_SLIDER);
	sync_single_standard_ur_transform(RGB_R_SLIDER_CURSOR);
	sync_single_standard_ur_transform(RGB_G_SLIDER);
	sync_single_standard_ur_transform(RGB_G_SLIDER_CURSOR);
	sync_single_standard_ur_transform(RGB_B_SLIDER);
	sync_single_standard_ur_transform(RGB_B_SLIDER_CURSOR);
	sync_single_standard_ur_transform(HSV_H_SLIDER);
	sync_single_standard_ur_transform(HSV_H_SLIDER_CURSOR);
	sync_single_standard_ur_transform(HSV_S_SLIDER);
	sync_single_standard_ur_transform(HSV_S_SLIDER_CURSOR);
	sync_single_standard_ur_transform(HSV_V_SLIDER);
	sync_single_standard_ur_transform(HSV_V_SLIDER_CURSOR);
	sync_single_standard_ur_transform(HSL_H_SLIDER);
	sync_single_standard_ur_transform(HSL_H_SLIDER_CURSOR);
	sync_single_standard_ur_transform(HSL_S_SLIDER);
	sync_single_standard_ur_transform(HSL_S_SLIDER_CURSOR);
	sync_single_standard_ur_transform(HSL_L_SLIDER);
	sync_single_standard_ur_transform(HSL_L_SLIDER_CURSOR);
	float sc = scale1d();
	if (cached_scale1d != sc)
	{
		cached_scale1d = sc;
		rr_wget(*this, BACKGROUND).update_corner_radius(sc).update_thickness(sc);
		for (size_t button : ALL_BUTTONS)
			b_t_wget(*this, button).update_corner_radius(sc).update_thickness(sc);
	}
	rr_wget(*this, BACKGROUND).update_transform().send_buffer();
	tr_wget(*this, TEXT_ALPHA).send_vp(*vp);
	tr_wget(*this, TEXT_RED).send_vp(*vp);
	tr_wget(*this, TEXT_GREEN).send_vp(*vp);
	tr_wget(*this, TEXT_BLUE).send_vp(*vp);
	tr_wget(*this, TEXT_HUE).send_vp(*vp);
	tr_wget(*this, TEXT_SAT).send_vp(*vp);
	tr_wget(*this, TEXT_VALUE).send_vp(*vp);
	tr_wget(*this, TEXT_LIGHT).send_vp(*vp);
	for (size_t button : ALL_BUTTONS)
		b_t_wget(*this, button).send_vp();

	gui_transform.scale = wp_at(BACKGROUND).transform.scale * Machine.get_app_scale() * self.transform.scale;
	gui_transform.position = Machine.to_screen_coordinates(children[BACKGROUND]->global_of({ -0.5f, 0.5f }), *vp);
	gui_transform.position.y = Machine.main_window->height() - gui_transform.position.y;
}

void ColorPicker::sync_single_standard_ur_transform(size_t control, bool send_buffer) const
{
	setup_vertex_positions(control);
	if (send_buffer)
		ur_wget(*this, control).send_buffer();
}

void ColorPicker::send_cpwc_buffer(size_t control) const
{
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

bool ColorPicker::cursor_in_bkg() const
{
	return children[BACKGROUND]->contains_screen_point(Machine.cursor_screen_pos(), *vp);
}
