/*
 * editor/editor_f.cpp
 *
 * Auslagerung von kleineren Funktionen, die in der Hauptschleife des Editors
 * (Datei editor/editor_c.cpp) aufgerufen werden
 *
 */

#include <climits>
#include "editor.h"

#include "../api/manager.h"
#include "../level/level_me.h"
#include "../level/obj_lib.h" // Suche nach Terrain/Spezialobjekten
#include "../other/user.h"

int Editor::get_grid()
{
    return panel[GRID_2 ].get_on() ?  2
         : panel[GRID_CUSTOM].get_on() ? useR->editor_grid_custom
         : panel[GRID_16].get_on() ? 16 : 1;
}



// Auswahl eines Grafikobjekts
Editor::Selection Editor::find_under_mouse_cursor(Editor::FindBy findby)
{
    Selection s;
    for (int type = Object::TERRAIN; type != Object::MAX; ++type) {
        // Normal durchlaufen,
        // Mit Rechtsklick andersherum durchlaufen
        int   perm      = Object::perm(type);
        GraIt itr       = object[perm].begin();
        bool  backwards = hardware.get_mrh();
        if (backwards) {
            perm = Object::perm(Object::MAX - type - 1);
            itr  = --object[perm].end();
        }
        while (itr != object[perm].end() && itr != --object[perm].begin()) {
            find_check(s, object[perm], itr, findby);
            if (backwards) --itr;
            else           ++itr;
        }
    }
    return s;
}



// Wird pro Objekt von find_under_mouse_cursor aufgerufen.
void Editor::find_check(Selection& s, GraLi& l, GraIt g, FindBy findby)
{
    const bool& tx  = level.torus_x;
    const bool& ty  = level.torus_y;
    const int&  txl = map.get_xl();
    const int&  tyl = map.get_yl();
                  find_check_at(s, l, g, findby, mx,       my);
    if (tx)       find_check_at(s, l, g, findby, mx + txl, my);
    if (ty)       find_check_at(s, l, g, findby, mx,       my + tyl);
    if (tx && ty) find_check_at(s, l, g, findby, mx + txl, my + tyl);
}



// Wird je nach Torus-Einstellung mehrmals von find_check aufgerufen, where_xy.
void Editor::find_check_at(
    Selection& s, GraLi& l, GraIt g, Editor::FindBy findby,
    const int whx, const int why
) {
    if (findby == Editor::FIND_BY_TRANSP) {
        const int col = g->get_pixel(whx - g->get_x(), why - g->get_y());
        if (col != color[COL_TRANSPARENT] && col != color[COL_PINK]) {
            s.o = g;
            s.l = &l;
        }
    }
    else {
        if (whx - g->get_x() >= g->get_selbox_x()
         && whx - g->get_x() <  g->get_selbox_x() + g->get_selbox_xl()
         && why - g->get_y() >= g->get_selbox_y()
         && why - g->get_y() <  g->get_selbox_y() + g->get_selbox_yl()) {
            s.o = g;
            s.l = &l;
        }
    }
}



// Hover-Hinzufuegung mehrerer Grafikobjekte beim Rahmenziehen (torussicher)
// Nutzt die Ueberlappungsfunktionen, die auch fuer die Vorder-/Hintergrund-
// angelegenheiten wichtig sind.
void Editor::find_check_frame
(GraLi& l, const int x1, const int y1, const int x2, const int y2)
{
    for (GraIt itr = l.begin(); itr != l.end(); ++itr)
     if (get_overlap(*itr, x1, y1, x2, y2, frame_torus_x, frame_torus_y))
     hover.push_back(Selection(&l, itr));
}



// Ueberlappung eines Grafikobjets mit einem Rechteck
// tx und ty sollten jeweils wahr sein, wenn das Rechteck ueber die Torus-
// -Nahtstelle geht.
bool Editor::get_overlap
(const EdGraphic& g, const int  x1, const int  y1, const int x2, const int y2,
                   const bool tx, const bool ty)
{
    const int x3 = map.get_xl() + 1;
    const int y3 = map.get_yl() + 1;
    if      (!tx && !ty) {
        return get_overlap_at(g, x1, y1, x2, y2);
    }
    else if ( tx && !ty) {
        return get_overlap_at(g, x2, y1, x3, y2)
         ||    get_overlap_at(g, -2, y1, x1, y2);
    }
    else if (!tx &&  ty) {
        return get_overlap_at(g, x1, y2, x2, y3)
         ||    get_overlap_at(g, x1, -2, x2, y1);
    }
    else { // tx && ty
        return get_overlap_at(g, x2, y2, x3, y3)
         ||    get_overlap_at(g, -2, y2, x1, y3)
         ||    get_overlap_at(g, x2, -2, x3, y1)
         ||    get_overlap_at(g, -2, -2, x1, y1);
    }
}

// Beim Torus auch verschoben suchen
bool Editor::get_overlap_at
(const EdGraphic& g, const int x1, const int y1, const int x2, const int y2)
{
    const int x3  = map.get_xl(); const bool tx = level.torus_x;
    const int y3  = map.get_yl(); const bool ty = level.torus_y;
    return           get_overlap_at_2(g, x1,    y1,    x2,    y2   )
     || (tx       && get_overlap_at_2(g, x1+x3, y1,    x2+x3, y2   ))
     || (ty       && get_overlap_at_2(g, x1,    y1+y3, x2,    y2+y3))
     || (tx && ty && get_overlap_at_2(g, x1+x3, y1+y3, x3+x3, y2+y3));
}

// Eigentliche Auswahl beim Rahmenziehen:
// Zur Auswahl reicht es, wenn ein kleiner Zipfel eines Objekts im Rahmen ist.
bool Editor::get_overlap_at_2
(const EdGraphic& g, const int x1, const int y1, const int x2, const int y2)
{
    const int top_x = g.get_x() + g.get_selbox_x();
    const int top_y = g.get_y() + g.get_selbox_y();
    const int bot_x = top_x     + g.get_selbox_xl();
    const int bot_y = top_y     + g.get_selbox_yl();
    if (bot_x > x1 && bot_y > y1 && top_x <= x2 && top_y <= y2)
         return true;
    else return false;
}



void Editor::delete_dublicates_from_selection()
{
    for (SelIt i = selection.begin(); i != selection.end(); ++i) {
        SelIt j = i;
        while (++j != selection.end()) if (*i == *j) selection.erase(j--);
}   }

void Editor::delete_from_selection(Selection& s)
{
    for (SelIt i =  --selection.end(); i != --selection.begin(); --i)
     if (*i == s) {
        selection.erase(i);
        break;
}   }



void Editor::selection_fore_background(const bool fg, const bool total)
{
    if (total) for (SelIt sel=selection.begin(); sel!=selection.end(); ++sel) {
        GraIt itr = fg ? sel->l->begin() : sel->l->end();
        sel->l->insert(itr, *sel->o);
        sel->l->erase(sel->o);
        sel->o = --itr; // das Eingeschobene
        draw_required = true;
    }
    else for (SelIt sel = selection.begin(); sel != selection.end(); ++sel) {
        if (sel->l == &object[Object::TERRAIN]) {
            const int x1 = sel->o->get_x();
            const int y1 = sel->o->get_y();
            const int x2 = sel->o->get_x() + sel->o->get_xl() - 1;
            const int y2 = sel->o->get_y() + sel->o->get_yl() - 1;
            GraIt itr = sel->o;
            if (fg) --itr;
            else    ++itr;
            for (; itr != (fg ? --(sel->l->begin()) : sel->l->end());
                           fg ? -- itr              : ++itr)
             if (get_overlap(*itr, x1, y1, x2, y2, false, false)) {
                // Die Auswahl vor dem Iterator einschieben. Wenn wir allerdings
                // in den Hintergrund stellen: hinter dem Iterator == vor ++itr.
                // ++itr.
                if (!fg) ++itr;
                sel->l->insert(itr, *sel->o);
                sel->l->erase(sel->o);
                sel->o = --itr; // das Eingeschobene
                draw_required = true;
                break;
            }
        }
        // In other lists move back/forth a single place without taking into
        // account overlapping objects
        else {
            GraIt itr = sel->o;
            if (fg) {
                if (itr == sel->l->begin() || sel->l->size() == 1) continue;
                --itr;
            }
            else {
                if (itr == --sel->l->end() || sel->l->size() == 1) continue;
                ++(++itr);
            }
            sel->l->insert(itr, *sel->o);
            sel->l->erase(sel->o);
            sel->o = --itr; // das Eingeschobene
            draw_required = true;
}   }   }


void Editor::delete_everything() {
    selection.clear();
    level    .clear();
    selection.clear();
    for (int type = Object::TERRAIN; type != Object::MAX; ++type)
     object[type].clear();

    useR->single_last_level
     = Filename(useR->single_last_level.get_dir_rootless());
    map.resize     (level.size_x, level.size_y);
    map.set_torus_x(level.torus_x);
    map.set_torus_y(level.torus_y);
    bg_color = makecol(level.bg_red, level.bg_green, level.bg_blue);

    draw_required = true;
}



// Dies erscheint beim Neu- oder Exit-Button
Api::BoxMessage* Editor::new_box_unsaved_data(const Level& l)
{
    Api::BoxMessage* box;

    if (useR->single_last_level.get_file().empty()) {
        box = new Api::BoxMessage(500, 2, Language::editor_unsaved_title_new);
        box->add_text(Language::editor_unsaved_question_new);
        box->add_text(Language::editor_level_name + ' ' + l.get_name());
    }
    else {
        std::string s1 = Language::editor_file_name  + ' '
                       + useR->single_last_level.get_rootless();
        std::string s2 = Language::editor_level_name + ' '
                       + LevelMetaData(useR->single_last_level).get_name();

        box = new Api::BoxMessage(500, 3, Language::editor_unsaved_title);
        box->add_text(Language::editor_unsaved_question);
        box->add_text(s1);
        box->add_text(s2);
    }
    box->add_button(Language::yes,    useR->key_me_okay);
    box->add_button(Language::no,     useR->key_me_delete);
    box->add_button(Language::cancel, useR->key_me_exit);

    return box;
}



bool Editor::search_criterion_terrain(const Filename& s) {
    if (s.get_extension() == gloB->ext_object_definitions) return false;
    const Object* ob = ObjLib::get(s);
    return ob && ob->type == Object::TERRAIN && ob->subtype != 1;
}
bool Editor::search_criterion_steel(const Filename& s) {
    if (s.get_extension() == gloB->ext_object_definitions) return false;
    const Object* ob = ObjLib::get(s);
    return ob && ob->type == Object::TERRAIN && ob->subtype == 1;
}
bool Editor::search_criterion_hatch(const Filename& s) {
    if (s.get_extension() == gloB->ext_object_definitions) return false;
    const Object* ob = ObjLib::get(s);
    return ob && ob->type == Object::HATCH;
}
bool Editor::search_criterion_goal(const Filename& s) {
    if (s.get_extension() == gloB->ext_object_definitions) return false;
    const Object* ob = ObjLib::get(s);
    return ob && ob->type == Object::GOAL;
}
bool Editor::search_criterion_deco(const Filename& s) {
    if (s.get_extension() == gloB->ext_object_definitions) return false;
    const Object* ob = ObjLib::get(s);
    return ob && ob->type == Object::DECO;
}
bool Editor::search_criterion_hazard(const Filename& s) {
    if (s.get_extension() == gloB->ext_object_definitions) return false;
    const Object* ob = ObjLib::get(s);
    return ob && (ob->type == Object::TRAP
     ||           ob->type == Object::WATER
     ||           ob->type == Object::FLING
     ||           ob->type == Object::TRAMPOLINE);
}


void Editor::flip_selection_individual() {
    for (SelIt i = selection.begin(); i != selection.end(); ++i) {
        if (i->l == &object[Object::TERRAIN])
         i->o->set_mirror(!i->o->get_mirror());
        if (i->l == &object[Object::HATCH])
         i->o->set_rotation(!i->o->get_rotation());
    }
}

void Editor::flip_selection() {
    int x_max = INT_MIN;
    int x_min = INT_MAX;
    for (SelIt i = selection.begin(); i != selection.end(); ++i) {
        x_max = std::max(x_max, i->o->get_x() + i->o->get_xl());
        x_min = std::min(x_min, i->o->get_x());
    }
    int x_pivot = (x_max + x_min) / 2;
    for (SelIt i = selection.begin(); i != selection.end(); ++i) {
        // move the piece according to the whole selection's ongoing flipping
        i->o->set_x(2 * x_pivot - i->o->get_x() - i->o->get_xl());
        // now rotate the piece individually
        if (i->l == &object[Object::TERRAIN]) {
            i->o->set_mirror(!i->o->get_mirror());
            if (i->o->get_rotation() == 0 || i->o->get_rotation() == 2)
                i->o->set_rotation(i->o->get_rotation() + 2);
        }
        if (i->l == &object[Object::HATCH])
            i->o->set_rotation(!i->o->get_rotation());
    }
}



void Editor::rotate_selection() {
    int y_max = INT_MIN;
    int y_min = INT_MAX;
    int x_max = INT_MIN;
    int x_min = INT_MAX;
    for (SelIt i = selection.begin(); i != selection.end(); ++i) {
        y_max = std::max(y_max, i->o->get_y() + i->o->get_yl());
        y_min = std::min(y_min, i->o->get_y());
        x_max = std::max(x_max, i->o->get_x() + i->o->get_xl());
        x_min = std::min(x_min, i->o->get_x());
    }
    int y_pivot = (y_max + y_min) / 2;
    int x_pivot = (x_max + x_min) / 2;
    for (SelIt i = selection.begin(); i != selection.end(); ++i) {
        int xl = i->o->get_xl();
        int dy = i->o->get_y() - y_pivot;
        int dx = i->o->get_x() - x_pivot;
        int y_new = y_pivot - dx - xl;
        int x_new = x_pivot + dy;
        i->o->set_y(y_new);
        i->o->set_x(x_new);
        if (i->l == &object[Object::TERRAIN])
            i->o->set_rotation(i->o->get_rotation() + 3);
        else if (i->l == &object[Object::HATCH])
            i->o->set_rotation(! i->o->get_rotation());
    }
}



void Editor::open_save_browser()
{
    browser_save = new Api::SaveBrowser(gloB->dir_levels,
                   gloB->ext_level,
                   useR->single_last_level,
                   Api::SaveBrowser::search_criterion_level,
                   Api::SaveBrowser::new_box_overwrite_level);
    browser_save->set_info_level_name(level.get_name());
    browser_save->set_info_filename(useR->single_last_level);
    Api::Manager::add_focus(browser_save);
}
