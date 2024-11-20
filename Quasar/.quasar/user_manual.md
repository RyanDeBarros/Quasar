# Quasar User Manual

Note on notation: ROW# refers to a # key on a keyboard's number row, whereas PAD# refers to a # key on a keyboard's numpad.

Key bindings in Quasar exist in a *control scheme*. Key bindings in the global control scheme (like CTRL+S to save) work in any specific control scheme. Some keys perform different actions depending on which control scheme is currently set.
The control scheme can be set from the dropdown in the main menu bar, or by the following global key bindings:
* FILE control scheme: CTRL+ROW1
* PALETTE control scheme: CTRL+ROW2

## Menu

### File

In FILE control scheme:
* Create new Quasar (.qua) file with CTRL+N.
* Open Quasar (.qua) file with CTRL+O.
* Import image into workstation with CTRL+I.
* Export to image with CTRL+E.

In global control scheme:
* Save with CTRL+S.
* Save as new file with CTRL+SHIFT+S.
* Save copy of file with CTRL+ALT+S.

### Edit

* Undo last change with CTRL+Z.
* Redo last undo with CTRL+SHIFT+Z.

### View

* Toggle minor gridlines with G.
* Toggle major gridlines with SHIFT+G.
* Note that you may not see gridlines if you are too far zoomed out. That is because if they were visible, they would cover the entire image.

## Navigation

### Panning

* Move mouse while holding MMB or SPACE+LMB.
* Hold SHIFT to only pan in one axis.
* Press ESCAPE to cancel panning.

### Zooming

* Scroll with mouse wheel or trackpad.
* Scroll slower by holding SHIFT.
* Scroll relative to center of canvas (not mouse cursor) by holding CTRL.

Pressing ROW0 will reset the camera to the default pan and zoom.

### Fullscreen/Maximize

* Maximize with ALT+ENTER.
* Fullscreen with F11.
* Exit maximize/fullscreen state with the same combo.

## Palette panel

### Color picker

* The primary/alternate color preview at the bottom is also a button. Left click it to switch to primary color picking, and right click it to switch to alternate color picking. They overlap, allowing the user to view how the two colors blend.
* Press SPACE+X to swap the primary and alternate colors specifically in the color picker.

### Color palette

* Left click on the color palette to select the primary color, and right click on the color palette to select the secondary color.
* Press the X key anywhere to swap the primary and alternate color selections.
* Scroll the mouse wheel or track pad to scroll through colors in a subscheme.
* Press the down arrow button to overwrite the primary selection with the current color in color picker.
* Press the + button to add a new color to the palette with the current color in color picker. If SHIFT is held, the new color will be inserted at the end of the palette instead of next to the primary selection. If CTRL is held, the new color will be selected as the primary color.
* Hold CTRL and left click on colors to remove them from the palette.
* Hold MMB or SPACE+LMB to move a color in the palette. Hold ALT to swap the color with every color your mouse passes over. If ALT is not held, then only the selected color will move, and the order of all other colors will be preserved.

If in PALETTE control scheme:
* O overwrites the primary selection. ALT+O overwrites the alternate selection.
* I inserts a new color from the primary selection. ALT+I inserts a new color from the alternate selection. The SHIFT and CTRL modifiers from the insert button still apply.
* DELETE deletes the primary selection. ALT+DELETE deletes the alternate selection.
* CTRL+N creates a new subpalette.
* CTRL+R renames the current subpalette.
* CTRL+D deletes the current subpalette.

## Brushes panel

### Brush tips

There are four brush tips to choose from: PENCIL, PEN, ERASER, and SELECT. When painting, PENCIL will blend the applied color with the existing underlying pixel color. PEN will replace the existing pixel, ERASE will ignore the selected color altogether and remove the underlying pixel. SELECT will add the pixel to the current canvas selection. The following key bindings are useful:

* ROW1: Select the PENCIL tool.
* ROW2: Select the PEN tool.
* ROW3: Select the ERASER tool.
* ROW4: Select the SELECT tool. Note that CTRL+A in any mode will select the entire canvas of pixels.

### Brush tools

There are several brush tools to choose from, which determine how pixels are interacted with in canvas, based on mouse input:

* Camera (key shortcut: Z) - Neutral mode that doesn't interact with the canvas aside from navigation.
* Paint (key shortcut: P) - Freehand draw on canvas.
* Line (key shortcut: L) - Click and drag to draw a controlled line.
* Fill (key shortcut: F) - Fill all connected (same color) pixels.
* Rect Outline (key shortcut: R) - Click and drag to draw a rectangle border.
* Rect Fill (key shortcut: SHIFT+R) - Click and drag to draw a filled-in rectangle.
* Ellipse Outline (key shortcut: E) - Click and drag to draw a ellipse border.
* Ellipse Fill (key shortcut: SHIFT+E) - Click and drag to draw a filled-in ellipse.
