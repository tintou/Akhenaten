#include "ornaments.h"

#include "building/building.h"
#include "building/model.h"
#include "building/dock.h"
#include "building/building_granary.h"
#include "building/building_entertainment.h"
#include "building/building_education.h"
#include "building/building_workshop.h"
#include "building/building_farm.h"
#include "building/monuments.h"
#include "building/building_animation.h"
#include "building/monument_mastaba.h"
#include "city/buildings.h"
#include "city/floods.h"
#include "city/labor.h"
#include "city/ratings.h"
#include "core/direction.h"
#include "game/time.h"
#include "graphics/graphics.h"
#include "graphics/image_desc.h"
#include "graphics/image.h"
#include "graphics/view/lookup.h"
#include "grid/floodplain.h"
#include "grid/image.h"
#include "grid/property.h"
#include "grid/random.h"
#include "grid/terrain.h"
#include <cmath>

/////// ANIMATIONS

void building_draw_normal_anim(painter &ctx, vec2i pixel, building *b, tile2i tile, const animation_t &anim, int color_mask) {
    if (anim.anim_id <= 0) {
        return;
    }
    int anim_id = image_group(anim.anim_id);
    int base_id = anim.base_id > 0 ? image_group(anim.base_id) : 0;

    building_draw_normal_anim(ctx, pixel + anim.pos, b, tile, anim_id, color_mask, base_id, anim.max_frames);
}

void building_draw_normal_anim(painter &ctx, vec2i pos, building* b, tile2i tile, int sprite_id, int color_mask, int base_id, int max_frames) {
    if (!sprite_id) {
        return;
    }

    int grid_offset = tile.grid_offset();
    if (!base_id) {
        base_id = map_image_at(grid_offset);
    }

    int animation_offset = building_animation_offset(b, base_id, grid_offset, max_frames);
    if (animation_offset == 0) {
        return;
    }

    if (base_id == sprite_id) {
        ImageDraw::img_ornament(ctx, sprite_id + animation_offset, base_id, pos.x, pos.y, color_mask);
    } else {
        ImageDraw::img_sprite(ctx, sprite_id + animation_offset, pos.x, pos.y, color_mask);
    }
}

static void draw_water_lift_anim(painter &ctx, building* b, int x, int y, color color_mask) {
    int orientation_rel = city_view_relative_orientation(b->data.industry.orientation);
    int anim_offset = 13 * orientation_rel;
    x += 53;
    y += 15;

    switch (orientation_rel) {
    case 1:
        y -= 2;
        break;
    case 2:
        x += 10;
        y += 0;
        break;
    case 3:
        x += 14;
        y += 6;
        break;
    }

    building_draw_normal_anim(ctx, vec2i{x, y}, b, b->tile, image_id_from_group(GROUP_WATER_LIFT_ANIM) - 1 + anim_offset, color_mask);
}

static void draw_fort_anim(int x, int y, building* b, painter &ctx) {
    if (map_property_is_draw_tile(b->tile.grid_offset())) {
        int offset = 0;
        switch (b->subtype.fort_figure_type) {
        case FIGURE_FCHARIOTEER:
            offset = 4;
            break;
        case FIGURE_INFANTRY:
            offset = 3;
            break;
        case FIGURE_ARCHER:
            offset = 2;
            break;
        }
        if (offset)
            ImageDraw::img_generic(ctx, image_id_from_group(GROUP_BUILDING_FORT) + offset, x + 81, y + 5, drawing_building_as_deleted(b) ? COLOR_MASK_RED : 0);
    }
}

static void draw_gatehouse_anim(int x, int y, building* b, painter &ctx) {
    int xy = map_property_multi_tile_xy(b->tile.grid_offset());
    int orientation = city_view_orientation();
    if ((orientation == DIR_0_TOP_RIGHT && xy == EDGE_X1Y1) || (orientation == DIR_2_BOTTOM_RIGHT && xy == EDGE_X0Y1)
        || (orientation == DIR_4_BOTTOM_LEFT && xy == EDGE_X0Y0)
        || (orientation == DIR_6_TOP_LEFT && xy == EDGE_X1Y0)) {
        int image_id = image_id_from_group(GROUP_BULIDING_GATEHOUSE);
        int color_mask = drawing_building_as_deleted(b) ? COLOR_MASK_RED : 0;
        if (b->subtype.orientation == 1) {
            if (orientation == DIR_0_TOP_RIGHT || orientation == DIR_4_BOTTOM_LEFT)
                ImageDraw::img_generic(ctx, image_id, x - 22, y - 80, color_mask);
            else
                ImageDraw::img_generic(ctx, image_id + 1, x - 18, y - 81, color_mask);
        } else if (b->subtype.orientation == 2) {
            if (orientation == DIR_0_TOP_RIGHT || orientation == DIR_4_BOTTOM_LEFT)
                ImageDraw::img_generic(ctx, image_id + 1, x - 18, y - 81, color_mask);
            else
                ImageDraw::img_generic(ctx, image_id, x - 22, y - 80, color_mask);
        }
    }
}

int get_farm_image(int grid_offset) {
    if (map_terrain_is(grid_offset, TERRAIN_FLOODPLAIN)) {
        int base = image_id_from_group(GROUP_BUILDING_FARMLAND);
        int fert_average = map_get_fertility_for_farm(grid_offset);
        int fertility_index = 0;

        if (fert_average < 13)
            fertility_index = 0;
        else if (fert_average < 25)
            fertility_index = 1;
        else if (fert_average < 38)
            fertility_index = 2;
        else if (fert_average < 50)
            fertility_index = 3;
        else if (fert_average < 63)
            fertility_index = 4;
        else if (fert_average < 75)
            fertility_index = 5;
        else if (fert_average < 87)
            fertility_index = 6;
        else
            fertility_index = 7;

        return base + fertility_index;
    } else {
        return image_id_from_group(GROUP_BUILDING_FARM_HOUSE);
    }
}

static void draw_dock_workers(building* b, int x, int y, color color_mask, painter &ctx) {
    int num_dockers = building_dock_count_idle_dockers(b);
    if (num_dockers > 0) {
        int image_dock = map_image_at(b->tile.grid_offset());
        int image_dockers = image_id_from_group(GROUP_BUILDING_DOCK_DOCKERS);
        if (image_dock == image_id_from_group(GROUP_BUILDING_DOCK))
            image_dockers += 0;
        else if (image_dock == image_id_from_group(GROUP_BUILDING_DOCK) + 1)
            image_dockers += 3;
        else if (image_dock == image_id_from_group(GROUP_BUILDING_DOCK) + 2)
            image_dockers += 6;
        else
            image_dockers += 9;

        if (num_dockers == 2)
            image_dockers += 1;
        else if (num_dockers == 3)
            image_dockers += 2;

        const image_t* img = image_get(image_dockers);
        ImageDraw::img_generic(ctx, image_dockers, x + img->animation.sprite_offset.x, y + img->animation.sprite_offset.y, color_mask);
    }
}

/////// ORNAMENTS

static void draw_hippodrome_ornaments(vec2i pixel, map_point point, painter &ctx) {
    int grid_offset = point.grid_offset();
    int x = pixel.x;
    int y = pixel.y;
    int image_id = map_image_at(grid_offset);
    const image_t* img = image_get(image_id);
    building* b = building_at(grid_offset);
    if (img->animation.num_sprites && map_property_is_draw_tile(grid_offset) && b->type == BUILDING_SENET_HOUSE) {
        ImageDraw::img_generic(ctx, image_id + 1, x + img->animation.sprite_offset.x, y + img->animation.sprite_offset.y - img->height + 90, drawing_building_as_deleted(b) ? COLOR_MASK_RED : 0);
    }
}

void draw_ornaments_flat(vec2i point, tile2i tile, painter &ctx) {
    int grid_offset = tile.grid_offset();
    // tile must contain image draw data
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }

    int image_id = map_image_at(grid_offset);
    building* b = building_at(grid_offset);

    if (b->type == 0 || b->state == BUILDING_STATE_UNUSED) {
        return;
    }

    // draw in red if necessary
    int color_mask = 0;
    if (drawing_building_as_deleted(b) || map_property_is_deleted(grid_offset)) {
        color_mask = COLOR_MASK_RED;
    }

    switch (b->type) {
    case BUILDING_SMALL_MASTABA:
    case BUILDING_SMALL_MASTABA_SIDE:
    case BUILDING_SMALL_MASTABA_WALL:
    case BUILDING_SMALL_MASTABA_ENTRANCE:
        draw_small_mastaba_anim_flat(ctx, point, b, color_mask);
        break;
    }
}

void draw_ornaments_and_animations_height(vec2i point, tile2i tile, painter &ctx) {
    int grid_offset = tile.grid_offset();
    // tile must contain image draw data
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }

    int image_id = map_image_at(grid_offset);
    building* b = building_at(grid_offset);
    if (b->type == BUILDING_STORAGE_YARD && b->state == BUILDING_STATE_CREATED) {
        ImageDraw::img_generic(ctx, image_id + 17, point.x - 5, point.y - 42);
    }

    if (b->type == 0 || b->state == BUILDING_STATE_UNUSED) {
        return;
    }

    // draw in red if necessary
    int color_mask = 0;
    if (drawing_building_as_deleted(b) || map_property_is_deleted(grid_offset)) {
        color_mask = COLOR_MASK_RED;
    }

    switch (b->type) {
    case BUILDING_BURNING_RUIN:
        building_draw_normal_anim(ctx, point, b, tile, image_id, color_mask);
        break;

    case BUILDING_DOCK:
        draw_dock_workers(b, point.x, point.y, color_mask, ctx);
        break;

    case BUILDING_WATER_LIFT:
        draw_water_lift_anim(ctx, b, point.x, point.y, color_mask);
        break;

    case BUILDING_STONE_QUARRY:
    case BUILDING_LIMESTONE_QUARRY:
    case BUILDING_GRANITE_QUARRY:
    case BUILDING_SANDSTONE_QUARRY:
        building_draw_normal_anim(ctx, point + vec2i{54, 15}, b, tile, image_group(ANIM_SANDSTONE_QUARRY_1) - 1, color_mask);
        break; 

    case BUILDING_SMALL_MASTABA:
    case BUILDING_SMALL_MASTABA_SIDE:
    case BUILDING_SMALL_MASTABA_WALL:
    case BUILDING_SMALL_MASTABA_ENTRANCE:
        draw_small_mastaba_anim(ctx, point, b, color_mask);
        break;

    case BUILDING_FORT_ARCHERS:
    case BUILDING_FORT_INFANTRY:
    case BUILDING_FORT_CHARIOTEERS:
        draw_fort_anim(point.x, point.y, b, ctx);
        break;

    case BUILDING_MUD_GATEHOUSE:
        draw_gatehouse_anim(point.x, point.y, b, ctx);
        break;

    case BUILDING_PAVILLION:
        if (map_image_at(grid_offset) == image_id_from_group(GROUP_BUILDING_PAVILLION)) {
            building_entertainment_draw_shows_dancers(ctx, b, point, color_mask);
        }
        break;

    case BUILDING_DANCE_SCHOOL:
        building_draw_normal_anim(ctx, point + vec2i{104, 0}, b, tile, image_id_from_group(GROUP_DANCERS_SHOW) - 1, color_mask);
        break;

    case BUILDING_FISHING_WHARF:
        building_draw_normal_anim(ctx, point + vec2i{74, 7}, b, tile, image_group(IMG_FISHIHG_WHARF_ANIM) - 1, color_mask);
        break;

    default:
        b->dcast()->draw_ornaments_and_animations_height(ctx, point, tile, color_mask);
        break;
    }

    // specific buildings
    building_workshop_draw_raw_material_storage(ctx, b, point, color_mask);
    building_education_draw_raw_material_storage(ctx, b, point, color_mask);
    //    draw_hippodrome_ornaments(pixel, point);
}