#include "ControlScheme.h"

#include "Machine.h"

static void global_key_handler_file(const KeyEvent& k)
{
	if (k.action == IAction::PRESS)
	{
		switch (k.key)
		{
		case Key::N:
			if (Machine.main_window->is_ctrl_pressed())
			{
				k.consumed = true;
				Machine.new_file();
			}
			break;
		case Key::O:
			if (Machine.main_window->is_ctrl_pressed())
			{
				k.consumed = true;
				Machine.open_file();
			}
			break;
		case Key::I:
			if (Machine.main_window->is_ctrl_pressed())
			{
				k.consumed = true;
				Machine.import_file();
			}
			break;
		case Key::E:
			if (Machine.main_window->is_ctrl_pressed())
			{
				k.consumed = true;
				Machine.export_file();
			}
			break;
		}
	}
}

static void global_key_handler_palette(const KeyEvent& k)
{
	if (k.action == IAction::PRESS)
	{
		switch (k.key)
		{
		case Key::I:
			Machine.palette_insert_color();
			break;
		case Key::O:
			Machine.palette_overwrite_color();
			break;
		case Key::DELETE:
			Machine.palette_delete_color();
			break;
		case Key::N:
			if (Machine.main_window->is_ctrl_pressed())
				Machine.palette_new_subpalette();
			break;
		case Key::R:
			if (Machine.main_window->is_ctrl_pressed())
				Machine.palette_rename_subpalette();
			break;
		case Key::D:
			if (Machine.main_window->is_ctrl_pressed())
				Machine.palette_delete_subpalette();
			break;
		}
	}
}

static void global_key_handler_neutral(const KeyEvent& k)
{
	if (k.action == IAction::PRESS)
	{
		switch (k.key)
		{
		case Key::ROW1:
			if (Machine.main_window->is_ctrl_pressed())
			{
				Machine.set_control_scheme(ControlScheme::FILE);
				k.consumed = true;
			}
			break;
		case Key::ROW2:
			if (Machine.main_window->is_ctrl_pressed())
			{
				Machine.set_control_scheme(ControlScheme::PALETTE);
				k.consumed = true;
			}
			break;
		case Key::ROW0:
			k.consumed = true;
			Machine.canvas_reset_camera();
			break;
		case Key::Z:
			if (Machine.main_window->is_ctrl_pressed())
			{
				k.consumed = true;
				if (Machine.main_window->is_shift_pressed())
				{
					if (Machine.redo_enabled())
					{
						Machine.start_held_redo();
						Machine.redo();
					}
				}
				else
				{
					if (Machine.undo_enabled())
					{
						Machine.start_held_undo();
						Machine.undo();
					}
				}
			}
			break;
		case Key::S:
			if (Machine.main_window->is_ctrl_pressed())
			{
				if (Machine.main_window->is_shift_pressed())
				{
					if (!Machine.main_window->is_alt_pressed())
					{
						k.consumed = true;
						Machine.save_file_as();
					}
				}
				else if (Machine.main_window->is_alt_pressed())
				{
					k.consumed = true;
					Machine.save_file_copy();
				}
				else
				{
					k.consumed = true;
					Machine.save_file();
				}
			}
			break;
		case Key::G:
			k.consumed = true;
			if (Machine.main_window->is_shift_pressed())
			{
				if (Machine.major_gridlines_visible())
					Machine.hide_major_gridlines();
				else
					Machine.show_major_gridlines();
			}
			else
			{
				if (Machine.minor_gridlines_visible())
					Machine.hide_minor_gridlines();
				else
					Machine.show_minor_gridlines();
			}
			break;
		case Key::ESCAPE:
			if (Machine.canvas_is_panning())
			{
				k.consumed = true;
				Machine.canvas_cancel_panning();
			}
			break;
		}
	}
}

void handle_global_key_event(const KeyEvent& k)
{
	global_key_handler_neutral(k);
	if (!k.consumed)
	{
		switch (Machine.get_control_scheme())
		{
		case ControlScheme::FILE:
			global_key_handler_file(k);
			break;
		case ControlScheme::PALETTE:
			global_key_handler_palette(k);
			break;
		}
	}
}
