#pragma once

#include "Panel.h"
#include "user/Platform.h"
#include "../widgets/Widget.h"
#include "../widgets/Button.h"

struct BrushesPanel : public Panel
{
	enum class BrushTip
	{
		PENCIL,
		PEN,
		ERASER,
		SELECT
	};

	enum class BrushTool
	{
		CAMERA,
		PAINT,
		LINE,
		FILL,
		RECT_OUTLINE,
		RECT_FILL,
		ELLIPSE_OUTLINE,
		ELLIPSE_FILL,
	};

private:
	BrushTip brush_tip = BrushTip::PENCIL;
	BrushTool brush_tool = BrushTool::CAMERA;

public:
	Shader bkg_shader, round_rect_shader;
	Widget widget;
	glm::mat3 vp;

	MouseButtonHandler mb_handler;
	KeyHandler key_handler;

	ToggleTButtonGroup toggle_group_tips;
	ToggleTButtonGroup toggle_group_tools;

private:
	float cached_scale1d = 0.0f;

public:
	BrushesPanel();
	BrushesPanel(const BrushesPanel&) = delete;
	BrushesPanel(BrushesPanel&&) noexcept = delete;

private:
	void initialize_widget();
	void connect_input_handlers();

public:
	virtual void draw() override;
	virtual void _send_view() override;
	virtual Scale minimum_screen_display() const override;

private:
	void sync_widget();
	void sync_ur(size_t subw);

public:
	void select_brush_tip(BrushTip tip);
	void select_brush_tool(BrushTool tool);

	void process();

	enum : size_t
	{
		BACKGROUND,
		EMPTY_BUTTON_TIP,
		EMPTY_BUTTON_TOOL,
		_W_COUNT
	};

	enum : size_t
	{
		BUTTON_TIP_PENCIL,
		BUTTON_TIP_PEN,
		BUTTON_TIP_ERASER,
		BUTTON_TIP_SELECT,
		_W_ETIP_COUNT
	};

	enum : size_t
	{
		BUTTON_TOOL_CAMERA,
		BUTTON_TOOL_PAINT,
		BUTTON_TOOL_LINE,
		BUTTON_TOOL_FILL,
		BUTTON_TOOL_RECT_OUTLINE,
		BUTTON_TOOL_RECT_FILL,
		BUTTON_TOOL_ELLIPSE_OUTLINE,
		BUTTON_TOOL_ELLIPSE_FILL,
		_W_ETOOL_COUNT
	};

private:
	// LATER implement empties elsewhere as well
	Widget& w_tip_parent();
	Widget& w_tool_parent();
};
