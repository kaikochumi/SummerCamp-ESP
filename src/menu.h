// menu.h — ESP and menu drawing.
#pragma once

#include "config.h"
#include "game.h"

namespace ui {

// Draws target boxes/names/bars into the background draw list.
void draw_esp(const Config& cfg, const game::Game& g, int view_w, int view_h);

// Draws the menu. Returns true if any value changed.
// out_rect receives the menu area (x0,y0,x1,y1) so we know when the
// cursor is over it and the overlay can accept clicks.
bool draw_menu(Config& cfg, game::Game& g, float out_rect[4]);

}  // namespace ui
