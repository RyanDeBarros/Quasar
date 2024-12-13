#include "Easel.h"

#include <glm/gtc/type_ptr.hpp>

#include "user/Machine.h"
#include "BrushesPanel.h"
#include "ImplUtility.h"
#include "../render/canvas/Canvas.h"
#include "../render/Uniforms.h"

Easel::Easel()
	: color_square_shader(FileSystem::shader_path("color_square.vert"), FileSystem::shader_path("color_square.frag")), widget(_W_COUNT)
{
	connect_input_handlers();
}

void Easel::initialize()
{
	initialize_widget();
}

void Easel::initialize_widget()
{
	assign_widget(&widget, BACKGROUND, std::make_shared<W_UnitRenderable>(&color_square_shader));
	ur_wget(widget, BACKGROUND).set_attribute(1, glm::value_ptr(RGBA(HSV(0.5f, 0.15f, 0.15f).to_rgb(), 0.5f).as_vec())).send_buffer();

	assign_widget(&widget, CANVAS, std::make_shared<Canvas>(&color_square_shader));
	Canvas& cnvs = canvas();
	cnvs.create_checkerboard_image();
	cnvs.set_image(nullptr);
	cnvs.checker1 = Machine.preferences.checker1;
	cnvs.checker2 = Machine.preferences.checker2;
	cnvs.set_checker_size(Machine.preferences.checker_size);
	cnvs.sync_checkerboard_colors();
}

void Easel::connect_input_handlers()
{
	mb_handler.callback = [this](const MouseButtonEvent& mb) {
		if (ImGui::GetIO().WantCaptureMouse)
			return;
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == IAction::PRESS && cursor_in_clipping())
			{
				mb.consumed = true;
				begin_panning();
			}
			else if (mb.action == IAction::RELEASE)
				end_panning();
		}
		else if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == IAction::PRESS && cursor_in_clipping())
			{
				if (MainWindow->is_key_pressed(Key::SPACE))
				{
					mb.consumed = true;
					begin_panning();
				}
				else if (MBrushes->get_brush_tool() & BrushTool::MOVE)
				{
					if (moving_mode == MovingMode::NONE)
					{
						if (begin_mouse_move_selection())
							mb.consumed = true;
					}
				}
				if (!mb.consumed)
				{
					mb.consumed = true;
					canvas().cursor_press(mb.button);
				}
			}
			else if (mb.action == IAction::RELEASE)
			{
				end_panning();
				end_mouse_move_selection();
				canvas().cursor_release(mb.button);
			}
		}
		else if (mb.button == MouseButton::RIGHT)
		{
			if (mb.action == IAction::PRESS)
			{
				mb.consumed = true;
				canvas().cursor_press(mb.button);
			}
			else if (mb.action == IAction::RELEASE)
				canvas().cursor_release(mb.button);
		}
		};
	// LATER add handler for when mouse is already pressed, and then space is pressed to pan.
	key_handler.callback = [this](const KeyEvent& k) {
		if (k.key == Key::LEFT_ALT || k.key == Key::RIGHT_ALT)
		{
			// no consume
			if (k.action == IAction::PRESS)
				canvas().transition_moving_selection_to_overwrite();
			else if (k.action == IAction::RELEASE)
				canvas().transition_moving_selection_to_blend();
		}
		if (k.action == IAction::PRESS)
		{
			if (k.key == Key::ESCAPE)
			{
				if (cancel_mouse_move_selection())
					k.consumed = true;
				else if (cancel_arrows_move_selection())
					k.consumed = true;
				else if (canvas().cursor_cancel())
					k.consumed = true;
				else if (canvas().deselect_all())
					k.consumed = true;
			}
			else if (k.mods & Mods::CONTROL)
			{
				if (k.key == Key::Z)
				{
					// no consume
					canvas().cursor_cancel();
				}
				else if (k.key == Key::A)
				{
					k.consumed = true;
					canvas().select_all();
				}
				else if (k.key == Key::D)
				{
					k.consumed = true;
					canvas().deselect_all();
				}
			}
			else if (k.key == Key::ENTER)
			{
				if (canvas().cursor_enter())
					k.consumed = true;
			}
			else if (k.key == Key::DELETE)
			{
				if (canvas().delete_selection())
					k.consumed = true;
			}
			else if (MainWindow->is_key_pressed(Key::SPACE))
			{
				if (k.key == Key::F)
				{
					k.consumed = true;
					if (k.mods & Mods::ALT)
						canvas().fill_selection_alternate();
					else
						canvas().fill_selection_primary();
				}
			}
			else if (moving_mode != MovingMode::MOUSE)
				handle_arrow_key_pressed(k);
		}
		else if (k.action == IAction::RELEASE)
		{
			if (moving_mode == MovingMode::ARROWS)
				handle_arrow_key_released(k);
		}
		};
	scroll_handler.callback = [this](const ScrollEvent& s) {
		if (!panning_info.panning && cursor_in_clipping())
		{
			s.consumed = true;
			zoom_by(s.yoff);
		}
		};
}

void Easel::handle_arrow_key_pressed(const KeyEvent& k)
{
	if (k.key != Key::LEFT && k.key != Key::RIGHT && k.key != Key::DOWN && k.key != Key::UP)
		return;
	if (moving_mode == MovingMode::NONE)
	{
		canvas().binfo.select_with_pixels = !MainWindow->is_key_pressed(Key::S);
		if (!canvas().batch_move_selection_start())
			return;
		moving_mode = MovingMode::ARROWS;
		arrows_move_sel_info.on_starting_interval = true;
		arrows_move_sel_info.held_time = 0.0f;
		arrows_move_sel_info.move_x = 0;
		arrows_move_sel_info.move_y = 0;
	}
	k.consumed = true;
	if (k.key == Key::LEFT)
		arrows_move_sel_info.left = true;
	else if (k.key == Key::RIGHT)
		arrows_move_sel_info.right = true;
	else if (k.key == Key::DOWN)
		arrows_move_sel_info.down = true;
	else if (k.key == Key::UP)
		arrows_move_sel_info.up = true;
	update_arrows_move_amounts();
	canvas().batch_move_selection_to(arrows_move_sel_info.move_x, arrows_move_sel_info.move_y);
}

void Easel::handle_arrow_key_released(const KeyEvent& k)
{
	if (k.key != Key::LEFT && k.key != Key::RIGHT && k.key != Key::DOWN && k.key != Key::UP)
		return;
	k.consumed = true;
	if (k.key == Key::LEFT)
		arrows_move_sel_info.left = false;
	else if (k.key == Key::RIGHT)
		arrows_move_sel_info.right = false;
	else if (k.key == Key::DOWN)
		arrows_move_sel_info.down = false;
	else if (k.key == Key::UP)
		arrows_move_sel_info.up = false;
	if (!arrows_move_sel_info.left && !arrows_move_sel_info.right && !arrows_move_sel_info.down && !arrows_move_sel_info.up)
	{
		canvas().batch_move_selection_submit();
		moving_mode = MovingMode::NONE;
	}
}

void Easel::draw()
{
	ur_wget(widget, BACKGROUND).draw();
	Canvas& cnvs = canvas();
	if (cnvs.visible)
		cnvs.draw();
}

void Easel::_send_view()
{
	vp = vp_matrix();
	canvas().send_vp(vp);
	Uniforms::send_matrix3(color_square_shader, "uVP", vp);
	unbind_shader();
	sync_widget();
}

void Easel::process()
{
	update_panning();
	update_mouse_move_selection();
	update_arrows_move_selection();
	canvas().process();
}

void Easel::sync_widget()
{
	widget.wp_at(BACKGROUND).transform.scale = get_app_size();
	sync_ur(BACKGROUND);
	canvas().sync_cursor_with_widget();
}

void Easel::sync_ur(size_t subw)
{
	Utils::set_vertex_pos_attributes(ur_wget(widget, subw), widget.wp_at(subw).relative_to(widget.self.transform));
}

void Easel::sync_canvas_transform()
{
	canvas().sync_transform();
	canvas().minor_gridlines.send_flat_transform(canvas().self.transform);
	canvas().major_gridlines.send_flat_transform(canvas().self.transform);
}

const Image* Easel::canvas_image() const
{
	return canvas().image.get();
}

Image* Easel::canvas_image()
{
	return canvas().image.get();
}

bool Easel::minor_gridlines_are_visible() const
{
	return canvas().minor_gridlines.visible();
}

void Easel::set_minor_gridlines_visibility(bool visible)
{
	canvas().minor_gridlines.set_visible(visible, canvas().self.transform, canvas().image->buf.width, canvas().image->buf.height);
}

bool Easel::major_gridlines_are_visible() const
{
	return canvas().major_gridlines.visible();
}

void Easel::set_major_gridlines_visibility(bool visible)
{
	canvas().major_gridlines.set_visible(visible, canvas().self.transform, canvas().image->buf.width, canvas().image->buf.height);
}

void Easel::begin_panning()
{
	if (!panning_info.panning)
	{
		panning_info.initial_canvas_pos = canvas().self.transform.position;
		panning_info.initial_cursor_pos = get_app_cursor_pos();
		panning_info.panning = true;
		MainWindow->request_cursor(&panning_info.wh, Machine.cursors.RESIZE_OMNI);
	}
}

void Easel::end_panning()
{
	if (panning_info.panning)
	{
		panning_info.panning = false;
		MainWindow->release_cursor(&panning_info.wh);
		MainWindow->release_mouse_mode(&panning_info.wh);
	}
}

void Easel::cancel_panning()
{
	if (panning_info.panning)
	{
		panning_info.panning = false;
		canvas().self.transform.position = panning_info.initial_canvas_pos;
		sync_canvas_transform();
		MainWindow->release_cursor(&panning_info.wh);
		MainWindow->release_mouse_mode(&panning_info.wh);
	}
}

void Easel::update_panning()
{
	if (panning_info.panning)
	{
		Position pan_delta = get_app_cursor_pos() - panning_info.initial_cursor_pos;
		Position pos = pan_delta + panning_info.initial_canvas_pos;
		if (MainWindow->is_shift_pressed())
		{
			if (std::abs(pan_delta.x) < std::abs(pan_delta.y))
				pos.x = panning_info.initial_canvas_pos.x;
			else
				pos.y = panning_info.initial_canvas_pos.y;
		}
		if (canvas().self.transform.position != pos)
		{
			canvas().self.transform.position = pos;
			sync_canvas_transform();
		}

		if (!MainWindow->owns_mouse_mode(&panning_info.wh) && !cursor_in_clipping())
			MainWindow->request_mouse_mode(&panning_info.wh, MouseMode::VIRTUAL);
		// LATER weirdly, virtual mouse is actually slower to move than visible mouse, so when virtual, scale deltas accordingly.
		// put factor in settings, and possibly even allow 2 speeds, with holding ALT or something.
	}
}

void Easel::zoom_by(float zoom)
{
	Position cursor_world;
	if (!MainWindow->is_ctrl_pressed())
		cursor_world = to_world_coordinates(Machine.cursor_screen_pos());

	float factor = MainWindow->is_shift_pressed() ? zoom_info.factor_shift : zoom_info.factor;
	float new_zoom = std::clamp(zoom_info.zoom * glm::pow(factor, zoom), zoom_info.in_min, zoom_info.in_max);
	float zoom_change = new_zoom / zoom_info.zoom;
	canvas().self.transform.scale *= zoom_change;
	Position delta_position = (canvas().self.transform.position - cursor_world) * zoom_change;
	canvas().self.transform.position = cursor_world + delta_position;

	sync_canvas_transform();
	zoom_info.zoom = new_zoom;
}

void Easel::reset_camera()
{
	zoom_info.zoom = zoom_info.initial;
	canvas().self.transform = {};
	if (canvas_image())
	{
		float fit_scale = std::min(get_app_width() / canvas_image()->buf.width, get_app_height() / canvas_image()->buf.height);
		if (fit_scale < 1.0f)
		{
			canvas().self.transform.scale *= fit_scale;
			zoom_info.zoom *= fit_scale;
		}
		else
		{
			fit_scale /= Machine.preferences.min_initial_image_window_proportion;
			if (fit_scale > 1.0f)
			{
				canvas().self.transform.scale *= fit_scale;
				zoom_info.zoom *= fit_scale;
			}
		}
	}
	sync_canvas_transform();
}

void Easel::flip_image_horizontally()
{
	struct FlipHorizontallyAction : public ActionBase
	{
		Easel* easel;
		FlipHorizontallyAction(Easel* easel) : easel(easel) { weight = sizeof(FlipHorizontallyAction); }
		virtual void forward() override { if (easel) easel->canvas_image()->flip_horizontally(); }
		virtual void backward() override { if (easel) easel->canvas_image()->flip_horizontally(); }
		QUASAR_ACTION_EQUALS_OVERRIDE(FlipHorizontallyAction)
	};
	struct FlipHorizontallyAction_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		FlipHorizontallyAction_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(FlipHorizontallyAction_Perf);
			buf = easel->canvas_image()->buf;
			buf.pxnew();
			subbuffer_copy(buf, easel->canvas_image()->buf);
			buf.flip_horizontally();
			weight += buf.bytes();
		}
		~FlipHorizontallyAction_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->update_texture();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(FlipHorizontallyAction_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<FlipHorizontallyAction_Perf>(this));
	else
		Machine.history.execute(std::make_shared<FlipHorizontallyAction>(this));
}

void Easel::flip_image_vertically()
{
	struct FlipVerticallyAction : public ActionBase
	{
		Easel* easel;
		FlipVerticallyAction(Easel* easel) : easel(easel) { weight = sizeof(FlipVerticallyAction); }
		virtual void forward() override { if (easel) easel->canvas_image()->flip_vertically(); }
		virtual void backward() override { if (easel) easel->canvas_image()->flip_vertically(); }
		QUASAR_ACTION_EQUALS_OVERRIDE(FlipVerticallyAction)
	};
	struct FlipVerticallyAction_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		FlipVerticallyAction_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(FlipVerticallyAction_Perf);
			buf = easel->canvas_image()->buf;
			buf.pxnew();
			subbuffer_copy(buf, easel->canvas_image()->buf);
			buf.flip_vertically();
			weight += buf.bytes();
		}
		~FlipVerticallyAction_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->update_texture();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(FlipVerticallyAction_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<FlipVerticallyAction_Perf>(this));
	else
		Machine.history.execute(std::make_shared<FlipVerticallyAction>(this));
}

void Easel::rotate_image_90()
{
	struct Rotate90Action : public ActionBase
	{
		Easel* easel;
		Rotate90Action(Easel* easel) : easel(easel) { weight = sizeof(Rotate90Action); }
		virtual void forward() override { if (easel) { easel->canvas_image()->rotate_90_del_old(); easel->canvas().sync_gfx_with_image(); } }
		virtual void backward() override { if (easel) { easel->canvas_image()->rotate_270_del_old(); easel->canvas().sync_gfx_with_image(); } }
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate90Action)
	};
	struct Rotate90Action_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		Rotate90Action_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(Rotate90Action_Perf);
			buf = easel->canvas_image()->buf.rotate_90_ret_new();
			weight += buf.bytes();
		}
		~Rotate90Action_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->resend_texture();
				easel->canvas().sync_gfx_with_image();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate90Action_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<Rotate90Action_Perf>(this));
	else
		Machine.history.execute(std::make_shared<Rotate90Action>(this));
}

void Easel::rotate_image_180()
{
	struct Rotate180Action : public ActionBase
	{
		Easel* easel;
		Rotate180Action(Easel* easel) : easel(easel) { weight = sizeof(Rotate180Action); }
		virtual void forward() override { if (easel) easel->canvas_image()->rotate_180(); }
		virtual void backward() override { if (easel) easel->canvas_image()->rotate_180(); }
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate180Action)
	};
	struct Rotate180Action_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		Rotate180Action_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(Rotate180Action_Perf);
			buf = easel->canvas_image()->buf;
			buf.pxnew();
			subbuffer_copy(buf, easel->canvas_image()->buf);
			buf.rotate_180();
			weight += buf.bytes();
		}
		~Rotate180Action_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->update_texture();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate180Action_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<Rotate180Action_Perf>(this));
	else
		Machine.history.execute(std::make_shared<Rotate180Action>(this));
}

void Easel::rotate_image_270()
{
	struct Rotate270Action : public ActionBase
	{
		Easel* easel;
		Rotate270Action(Easel* easel) : easel(easel) { weight = sizeof(Rotate270Action); }
		virtual void forward() override { if (easel) { easel->canvas_image()->rotate_270_del_old(); easel->canvas().sync_gfx_with_image(); } }
		virtual void backward() override { if (easel) { easel->canvas_image()->rotate_90_del_old(); easel->canvas().sync_gfx_with_image(); } }
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate270Action)
	};
	struct Rotate270Action_Perf : public ActionBase
	{
		Easel* easel;
		Buffer buf;
		Rotate270Action_Perf(Easel* easel) : easel(easel)
		{
			weight = sizeof(Rotate270Action_Perf);
			buf = easel->canvas_image()->buf.rotate_270_ret_new();
			weight += buf.bytes();
		}
		~Rotate270Action_Perf() { delete[] buf.pixels; }
		virtual void forward() override { execute(); }
		virtual void backward() override { execute(); }
		void execute()
		{
			if (easel)
			{
				std::swap(buf, easel->canvas_image()->buf);
				easel->canvas_image()->resend_texture();
				easel->canvas().sync_gfx_with_image();
			}
		}
		QUASAR_ACTION_EQUALS_OVERRIDE(Rotate270Action_Perf)
	};
	if (image_edit_perf_mode)
		Machine.history.execute(std::make_shared<Rotate270Action_Perf>(this));
	else
		Machine.history.execute(std::make_shared<Rotate270Action>(this));
}

bool Easel::begin_mouse_move_selection()
{
	canvas().binfo.select_with_pixels = !MainWindow->is_key_pressed(Key::S);
	if (canvas().batch_move_selection_start())
	{
		moving_mode = MovingMode::MOUSE;
		mouse_move_sel_info.initial_cursor_pos = get_app_cursor_pos();
		mouse_move_sel_info.initial_canvas_pos = canvas().self.transform.position;
		return true;
	}
	return false;
}

void Easel::end_mouse_move_selection()
{
	if (moving_mode == MovingMode::MOUSE)
	{
		moving_mode = MovingMode::NONE;
		canvas().batch_move_selection_submit();
		MainWindow->release_mouse_mode(&mouse_move_sel_info.wh);
	}
}

bool Easel::cancel_mouse_move_selection()
{
	if (moving_mode == MovingMode::MOUSE)
	{
		moving_mode = MovingMode::NONE;
		canvas().batch_move_selection_cancel();
		MainWindow->release_mouse_mode(&mouse_move_sel_info.wh);
		return true;
	}
	return false;
}

// TODO if mouse re-enters clipping, display mouse again. For moving selection AND panning

void Easel::update_mouse_move_selection()
{
	if (moving_mode == MovingMode::MOUSE)
	{
		Position pan_delta = get_app_cursor_pos() - mouse_move_sel_info.initial_cursor_pos;
		Position pos = pan_delta + mouse_move_sel_info.initial_canvas_pos;
		if (MainWindow->is_shift_pressed())
		{
			if (std::abs(pan_delta.x) < std::abs(pan_delta.y))
				pos.x = mouse_move_sel_info.initial_canvas_pos.x;
			else
				pos.y = mouse_move_sel_info.initial_canvas_pos.y;
		}

		canvas().batch_move_selection_to(pos.x - mouse_move_sel_info.initial_canvas_pos.x, pos.y - mouse_move_sel_info.initial_canvas_pos.y);

		if (!MainWindow->owns_mouse_mode(&mouse_move_sel_info.wh) && !cursor_in_clipping())
			MainWindow->request_mouse_mode(&mouse_move_sel_info.wh, MouseMode::CAPTURED);
	}
}

void Easel::update_arrows_move_selection()
{
	if (moving_mode == MovingMode::ARROWS)
	{
		arrows_move_sel_info.held_time += arrows_move_sel_info.held_speed_factor * Machine.delta_time();
		if (arrows_move_sel_info.on_starting_interval)
		{
			if (arrows_move_sel_info.held_time > arrows_move_sel_info.held_start_interval)
			{
				arrows_move_sel_info.held_time -= arrows_move_sel_info.held_start_interval;
				update_arrows_move_amounts();
				while (arrows_move_sel_info.held_time > arrows_move_sel_info.held_interval)
				{
					arrows_move_sel_info.held_time -= arrows_move_sel_info.held_interval;
					update_arrows_move_amounts();
				}
				canvas().batch_move_selection_to(arrows_move_sel_info.move_x, arrows_move_sel_info.move_y);
				arrows_move_sel_info.on_starting_interval = false;
			}
		}
		else if (arrows_move_sel_info.held_time > arrows_move_sel_info.held_interval)
		{
			do
			{
				arrows_move_sel_info.held_time -= arrows_move_sel_info.held_interval;
				update_arrows_move_amounts();
			}
			while (arrows_move_sel_info.held_time > arrows_move_sel_info.held_interval);
			canvas().batch_move_selection_to(arrows_move_sel_info.move_x, arrows_move_sel_info.move_y);
		}
	}
}

void Easel::update_arrows_move_amounts()
{
	if (arrows_move_sel_info.left)
		--arrows_move_sel_info.move_x;
	if (arrows_move_sel_info.right)
		++arrows_move_sel_info.move_x;
	if (arrows_move_sel_info.down)
		--arrows_move_sel_info.move_y;
	if (arrows_move_sel_info.up)
		++arrows_move_sel_info.move_y;
}

bool Easel::cancel_arrows_move_selection()
{
	if (moving_mode == MovingMode::ARROWS)
	{
		moving_mode = MovingMode::NONE;
		canvas().batch_move_selection_cancel();
		return true;
	}
	return false;
}

Canvas& Easel::canvas()
{
	return *widget.get<Canvas>(CANVAS);
}

const Canvas& Easel::canvas() const
{
	return *widget.get<Canvas>(CANVAS);
}
