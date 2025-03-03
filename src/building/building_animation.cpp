#include "building/building_type.h"
#include "grid/water_supply.h"

#include "building/industry.h"
#include "core/calc.h"
#include "core/game_environment.h"
#include "graphics/image.h"
#include "game/game.h"
#include "grid/sprite.h"

int generic_sprite_offset(int grid_offset, int max_frames, int anim_speed) {
    //    const image *img = image_get(image_id);
    //    if (!max_frames)
    //        max_frames = img->animation.num_sprites;
    //    int anim_speed = img->animation.speed_id;
    if (!game.animation_should_advance(anim_speed))
        return map_sprite_animation_at(grid_offset) & 0x7f;

    // advance animation
    int new_sprite = 0;
    //    bool is_reverse = false;
    //    if (img->animation.can_reverse) {
    //        if (map_sprite_animation_at(grid_offset) & 0x80)
    //            is_reverse = true;
    //
    //        int current_sprite = map_sprite_animation_at(grid_offset) & 0x7f;
    //        if (is_reverse) {
    //            new_sprite = current_sprite - 1;
    //            if (new_sprite < 1) {
    //                new_sprite = 1;
    //                is_reverse = false;
    //            }
    //        } else {
    //            new_sprite = current_sprite + 1;
    //            if (new_sprite > max_frames) {
    //                new_sprite = max_frames;
    //                is_reverse = true;
    //            }
    //        }
    //    } else // Absolutely normal case
    new_sprite = map_sprite_animation_at(grid_offset) + 1;
    if (new_sprite > max_frames)
        new_sprite = 1;

    map_sprite_animation_set(grid_offset, new_sprite);
    return new_sprite;
}

int building_animation_offset(building* b, int image_id, int grid_offset, int max_frames) {
    if (building_is_workshop(b->type) && (!b->workshop_has_resources() || b->num_workers <= 0)) {
        return 0;
    }

    switch (b->type) {
    case BUILDING_BURNING_RUIN:
        break;

    case BUILDING_STORAGE_ROOM:
        return 0;
        break;

    case BUILDING_MENU_BEAUTIFICATION:
        if (b->num_workers <= 0 || !b->has_water_access) {
            return 0;
        }
        break;

    case BUILDING_WATER_LIFT:
        if (b->num_workers <= 0) {
           return 0;
        } else if (!b->has_water_access) {
            return 0;
        }
        break;

    case BUILDING_PERSONAL_MANSION:
        break;

    case BUILDING_WELL:
        if (map_water_supply_is_well_unnecessary(b->id, 3) != WELL_NECESSARY) {
            return 0;
        }
        break;
        //        case BUILDING_PREFECTURE: // police house
        //        case BUILDING_ARCHITECT_POST:
        //        case BUILDING_FIREHOUSE:
        //        case BUILDING_WATER_SUPPLY:
        //        case BUILDING_PHYSICIAN:
        //        case BUILDING_APOTHECARY:
        //        case BUILDING_DENTIST:
        //        case BUILDING_MORTUARY:
        //        case BUILDING_BAZAAR:
        //        case BUILDING_STORAGE_YARD: // b->num_workers < model_get_building(b->type)->laborers
        //        case BUILDING_GRANARY: // b->num_workers < model_get_building(b->type)->laborers
        //        case BUILDING_IRON_MINE:
        //        case BUILDING_CLAY_PIT:
        //        case BUILDING_TIMBER_YARD:
        //        case BUILDING_THEATER:
        //        case BUILDING_CHARIOT_MAKER:
        //        case BUILDING_HIPPODROME:
        //        case BUILDING_TEMPLE_OSIRIS:
        //        case BUILDING_TEMPLE_RA:
        //        case BUILDING_TEMPLE_SETH:
        //        case BUILDING_TEMPLE_PTAH:
        //        case BUILDING_TEMPLE_BAST:
        //        case BUILDING_VILLAGE_PALACE:
        //        case BUILDING_TOWN_PALACE:
        //        case BUILDING_CITY_PALACE:
        //            if (b->num_workers <= 0)
        //                return 0;
        //            break;
    case BUILDING_STONE_QUARRY:
    case BUILDING_CONSERVATORY:
        if (b->num_workers <= 0) {
            return 0;
            //map_sprite_animation_set(grid_offset, 1);
            //return 1;
        }
        break;

    case BUILDING_DOCK:
        if (b->data.dock.num_ships <= 0) {
            map_sprite_animation_set(grid_offset, 1);
            return 1;
        }
        break;

    default:
        if (b->main()->num_workers <= 0) {
            return 0;
        }
        break;
    }

    const image_t* img = image_get(image_id);
    if (!max_frames) {
        max_frames = img->animation.num_sprites;
    }

    int anim_speed = b->dcast()->animation_speed(img->animation.speed_id);

    if (!game.animation_should_advance(anim_speed)) {
        return map_sprite_animation_at(grid_offset) & 0x7f;
    }

    // advance animation
    int new_sprite = 0;
    bool is_reverse = false;
    if (img->animation.can_reverse) {
        if (map_sprite_animation_at(grid_offset) & 0x80)
            is_reverse = true;

        int current_sprite = map_sprite_animation_at(grid_offset) & 0x7f;
        if (is_reverse) {
            new_sprite = current_sprite - 1;
            if (new_sprite < 1) {
                new_sprite = 1;
                is_reverse = false;
            }
        } else {
            new_sprite = current_sprite + 1;
            if (new_sprite > max_frames) {
                new_sprite = max_frames;
                is_reverse = true;
            }
        }
    } else {
        // Absolutely normal case
        new_sprite = map_sprite_animation_at(grid_offset) + 1;
        if (new_sprite > max_frames) {
            new_sprite = 1;
        }
    }

    map_sprite_animation_set(grid_offset, is_reverse ? new_sprite | 0x80 : new_sprite);
    return new_sprite;
}