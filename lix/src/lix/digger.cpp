/*
 * lemming/digger.cpp
 *
 * special_x
 *
 *   This is 1 if the digger has just been activated. Some earth will
 *   be dug away over its head on the first swing.
 *
 * special_y
 *
 *   This is 1 if the lix shall stop digging on hitting a single steel pixel.
 *   This is 0 otherwise. See also the comment near set_special_y(1).
 *
 */

#include "ac.h"

void assign_digger(Lixxie& l)
{
    l.assign_default(LixEn::DIGGER);
    l.set_special_x(1);
    l.set_special_y(0);
}



void update_digger(Lixxie& l, const UpdateArgs& ua)
{
    int steel_pixels_hit = 0;

    switch (l.get_frame()) {
    case 0:
        // This is the only steel check in the whole cycle of 16 frames.
        // Stop digging if there is steel to the left as well as to the right
        // of the digger's center, or immediately in the center.
        if (l.count_steel(-8, 2, 1, 5)
         && l.count_steel( 0, 2, 9, 5)) {
            l.set_special_y(1);
        }
        steel_pixels_hit += l.remove_rectangle(6, 2, 9, 3);
        steel_pixels_hit += l.remove_rectangle(7, 4, 9, 4);
        steel_pixels_hit += l.remove_rectangle(8, 5, 9, 5);
        break;
    case 1:
        steel_pixels_hit += l.remove_rectangle(3, 2, 5, 2);
        steel_pixels_hit += l.remove_rectangle(4, 3, 5, 3);
        steel_pixels_hit += l.remove_rectangle(4, 4, 6, 4);
        steel_pixels_hit += l.remove_rectangle(5, 5, 7, 5);
        break;
    case 2:
        steel_pixels_hit += l.remove_rectangle(0, 2, 2, 2);
        steel_pixels_hit += l.remove_rectangle(0, 3, 3, 4);
        steel_pixels_hit += l.remove_rectangle(0, 5, 4, 5);
        // this frame will later have protection from falling. You can assign
        // building skills in midair here, but builder and platformer will
        // still connect to the partly digged wall.
        break;
    case 3:
        steel_pixels_hit += l.remove_rectangle(-4, 2, -1, 5);
        // This is the only frame of downward motion
        l.move_down(4);
        break;
    case 4:
        // Since we have moved down, we must adjust the y coordinates
        steel_pixels_hit += l.remove_rectangle(-6, -2, -5, 1);
        break;
    case 5:
        steel_pixels_hit += l.remove_rectangle(-8, -2, -7, 1);
        break;
    }
    // done with the switch across the different frames

    if (steel_pixels_hit && l.get_special_y() == 1) {
        // Steel was hit and we actually want to stop
        l.turn();
        l.assign(LixEn::WALKER);
        l.set_frame(1);
        l.play_sound(ua, Sound::STEEL);
    }
    else if (!l.is_solid() && !l.is_solid(-2, 2) && !l.is_solid(2, 2)
             && l.get_frame() != 2) {
        l.assign(LixEn::FALLER);
    }
    else {
        // Stay digger
        l.next_frame();
        // Remove some earth over the foot if the lix was assigned recently
        if (l.get_special_x() == 1) {
            l.set_special_x(0);
            l.remove_rectangle(-8, -4, 9, 1);
        }
    }

}
