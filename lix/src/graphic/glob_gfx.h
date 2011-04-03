/*
 * glob_gfx.h
 *
 * bool clear_screen_at_next_blit
 *
 *   Globale Variable, die, falls sie gesetzt ist, beim naechsten Blit auf
 *   den Bildschirm zurueckgesetzt wird. War sie gesetzt, dann wird der
 *   Bildschirm zunaechst schwarz gefuellt, bevor auf ihn geblittet wird.
 *   Das macht Sinn, wenn man die Ganzzahlvergroesserung oder das Seiten-
 *   verhaeltnis ausstellt: Sonst bleibt statt des Trauerrandes ein Rest vom
 *   Optionsmenue haengen.
 *
 *   Das kurze Geflackere beim Hauptmenue-Erscheinen wird dabei in Kauf
 *   genommen. Deshalb sollte im Optionsmenue wirklich nur bei Bedarf diese
 *   Variable gesetzt werden!
 *
 */

#pragma once

#include <string>
#include <map>
#include <vector>

#include "cutbit.h"
#include "torbit.h"

extern ALLEGRO_DISPLAY* displaY;

extern Torbit *pre_screen;

static const int LEMSCR_X = 640;
static const int LEMSCR_Y = 480;

// Farben
enum Color {
    COL_NOTHING,
    COL_TRANSPARENT,
    COL_PINK,
    COL_REALPINK,
    COL_PINKAF,

    COL_BLACK,
    COL_GREY_FUSE_D,
    COL_GREY_FUSE_L,
    COL_WHITE,
    COL_YELLOW,
    COL_STEEL_MASK,

    COL_SCREEN_BORDER,
    COL_EDITOR_DARK,
    COL_API_SHADOW,
    COL_API_D,
    COL_API_M,
    COL_API_L,
    COL_API_DOWN_D,
    COL_API_DOWN_M,
    COL_API_DOWN_L,
    COL_API_ON_D,
    COL_API_ON_M,
    COL_API_ON_L,

    COL_TEXT,
    COL_TEXT_ON,
    COL_TEXT_OFF,
    COL_TEXT_GREEN,

    COL_LIXFILE_EYE,

    COL_MAX
};

extern std::vector <ALLEGRO_COLOR> color;

bool operator == (const ALLEGRO_COLOR&, const ALLEGRO_COLOR&);
bool operator != (const ALLEGRO_COLOR&, const ALLEGRO_COLOR&);

// Schriftarten
extern ALLEGRO_FONT *font_sml;
extern ALLEGRO_FONT *font_med;
extern ALLEGRO_FONT *font_nar;
extern ALLEGRO_FONT *font_big;

// Und die Funktionen
void load_all_bitmaps();
void destroy_all_bitmaps();

extern bool clear_screen_at_next_blit;
void blit_to_screen(ALLEGRO_BITMAP*);

// Will use global variables if res == (0, 0)
void set_screen_mode(bool full, int res_x = 0, int res_y = 0);
