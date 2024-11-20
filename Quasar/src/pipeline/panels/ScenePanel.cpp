#include "ScenePanel.h"

#include "user/Machine.h"

void ScenePanel::draw()
{
}

void ScenePanel::_send_view()
{
}

Scale ScenePanel::minimum_screen_display() const
{
    // LATER
    return { 0, Machine.window_layout_info.scene_panel_height };
}
