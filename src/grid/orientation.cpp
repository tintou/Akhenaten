#include "orientation.h"

#include "building.h"
#include "building/rotation.h"
#include "building/monuments.h"
#include "building/construction/build_planner.h"
#include "building/monument_mastaba.h"
#include "building/building_statue.h"
#include "core/direction.h"
#include "core/log.h"
#include "figuretype/animal.h"
#include "figuretype/wall.h"
#include "game/undo.h"
#include "graphics/image.h"
#include "graphics/image_groups.h"
#include "graphics/view/view.h"
#include "grid/bridge.h"
#include "grid/building_tiles.h"
#include "grid/grid.h"
#include "grid/property.h"
#include "grid/routing/routing_terrain.h"
#include "grid/terrain.h"
#include "grid/tiles.h"
#include "grid/water.h"
#include "image.h"
#include <scenario/map.h>

static void determine_leftmost_tile() {
    int orientation = city_view_orientation();
    int grid_offset = scenario_map_data()->start_offset;
    for (int y = 0; y < scenario_map_data()->height; y++, grid_offset += scenario_map_data()->border_size) {
        for (int x = 0; x < scenario_map_data()->width; x++, grid_offset++) {
            int size = map_property_multi_tile_size(grid_offset);
            if (size == 1) {
                map_property_mark_draw_tile(grid_offset);
                continue;
            }
            map_property_clear_draw_tile(grid_offset);
            int dx = orientation == DIR_4_BOTTOM_LEFT || orientation == DIR_6_TOP_LEFT ? size - 1 : 0;
            int dy = orientation == DIR_0_TOP_RIGHT || orientation == DIR_6_TOP_LEFT ? size - 1 : 0;
            if (map_property_is_multi_tile_xy(grid_offset, dx, dy))
                map_property_mark_draw_tile(grid_offset);
        }
    }
}

void map_orientation_change(int counter_clockwise) {
    map_tiles_remove_entry_exit_flags();
    game_undo_disable();
    determine_leftmost_tile();

    map_tiles_update_all_elevation();
    map_tiles_river_refresh_entire();
    map_tiles_update_all_earthquake();
    map_tiles_update_all_rocks();
    map_tiles_update_all_gardens();

    map_tiles_add_entry_exit_flags();

    map_tiles_update_all_empty_land();
    map_tiles_update_all_meadow();
    map_tiles_update_all_vegetation_tiles();
    map_tiles_update_all_rubble();
    map_tiles_update_all_roads();
    map_tiles_update_all_plazas();
    map_tiles_update_all_walls();
    map_tiles_update_all_aqueducts(0);

    map_orientation_update_buildings();
    map_bridge_update_after_rotate(counter_clockwise);
    map_routing_update_walls();

    figure_tower_sentry_reroute();
    figure_hippodrome_horse_reroute();
    Planner.update_orientations(false);
}

void map_orientation_update_buildings(void) {
    int map_orientation = city_view_orientation();
    int orientation_is_top_bottom = map_orientation == DIR_0_TOP_RIGHT || map_orientation == DIR_4_BOTTOM_LEFT;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_UNUSED)
            continue;

        int image_id;
        int image_offset;
        switch (b->type) {
        case BUILDING_MUD_GATEHOUSE:
            if (b->subtype.orientation == 1) {
                if (orientation_is_top_bottom)
                    image_id = image_id_from_group(GROUP_BUILDING_TOWER) + 1;
                else {
                    image_id = image_id_from_group(GROUP_BUILDING_TOWER) + 2;
                }
            } else {
                if (orientation_is_top_bottom)
                    image_id = image_id_from_group(GROUP_BUILDING_TOWER) + 2;
                else {
                    image_id = image_id_from_group(GROUP_BUILDING_TOWER) + 1;
                }
            }
            map_building_tiles_add(i, b->tile, b->size, image_id, TERRAIN_GATEHOUSE | TERRAIN_BUILDING);
            map_terrain_add_gatehouse_roads(b->tile.x(), b->tile.y(), 0);
            break;
        case BUILDING_RESERVED_TRIUMPHAL_ARCH_56:
            if (b->subtype.orientation == 1) {
                if (orientation_is_top_bottom)
                    image_id = image_id_from_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
                else {
                    image_id = image_id_from_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
                }
            } else {
                if (orientation_is_top_bottom)
                    image_id = image_id_from_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
                else {
                    image_id = image_id_from_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
                }
            }
            map_building_tiles_add(i, b->tile, b->size, image_id, TERRAIN_BUILDING);
            map_terrain_add_triumphal_arch_roads(b->tile.x(), b->tile.y(), b->subtype.orientation);
            break;
        case BUILDING_SHIPWRIGHT:
            image_offset = city_view_relative_orientation(b->data.industry.orientation);
            image_id = image_id_from_group(GROUP_BUILDING_SHIPYARD) + image_offset;
            map_water_add_building(i, b->tile, 2, image_id);
            break;
        case BUILDING_FISHING_WHARF:
            image_offset = city_view_relative_orientation(b->data.industry.orientation);
            image_id = image_id_from_group(GROUP_BUILDING_FISHING_WHARF) + image_offset;
            map_water_add_building(i, b->tile, 2, image_id);
            break;
        case BUILDING_FERRY:
            image_offset = city_view_relative_orientation(b->data.industry.orientation);
            image_id = image_id_from_group(GROUP_BUILDING_FERRY) + image_offset;
            map_water_add_building(i, b->tile, 2, image_id);
            break;
        case BUILDING_DOCK:
            image_offset = city_view_relative_orientation(b->data.dock.orientation);
            image_id = image_id_from_group(GROUP_BUILDING_DOCK) + image_offset;
            map_water_add_building(i, b->tile, 3, image_id);
            break;
        case BUILDING_WATER_LIFT:
            image_offset = city_view_relative_orientation(b->data.industry.orientation);
            if (!map_terrain_exists_tile_in_radius_with_type(b->tile, 2, 1, TERRAIN_WATER)) {
                image_offset += 4;
            } else if (map_terrain_exists_tile_in_radius_with_type(b->tile, 2, 1, TERRAIN_FLOODPLAIN)) {
                image_offset += 8;
            }
            image_id = image_id_from_group(GROUP_BUILDING_WATER_LIFT) + image_offset;
            map_water_add_building(i, b->tile, 2, image_id);
            break;
        case BUILDING_BOOTH:
        case BUILDING_BANDSTAND:
        case BUILDING_PAVILLION:
        case BUILDING_FESTIVAL_SQUARE:
            // only update the plaza tiles for the main venue
            if (b->is_main()) {
                int size = -1;
                int plaza_image_id = 0;
                switch (b->type) {
                case BUILDING_BOOTH:
                    size = 2;
                    plaza_image_id = image_group(IMG_BOOTH_SQUARE);
                    break;

                case BUILDING_BANDSTAND:
                    size = 3;
                    plaza_image_id = image_group(IMG_BANDSTAND_SQUARE);
                    break;

                case BUILDING_PAVILLION:
                    size = 4;
                    plaza_image_id = image_id_from_group(GROUP_PAVILLION_SQUARE);
                    break;

                case BUILDING_FESTIVAL_SQUARE:
                    size = 5;
                    plaza_image_id = image_id_from_group(GROUP_FESTIVAL_SQUARE);
                    break;
                }
                map_add_venue_plaza_tiles(b->id, size, MAP_X(b->data.entertainment.booth_corner_grid_offset), MAP_Y(b->data.entertainment.booth_corner_grid_offset), plaza_image_id, true);
            }
            // additionally, correct bandstand graphics
            if (b->type == BUILDING_BANDSTAND) {
                map_add_bandstand_tiles(b);
            }
            break;
        case BUILDING_SMALL_MASTABA:
        case BUILDING_SMALL_MASTABA_ENTRANCE:
        case BUILDING_SMALL_MASTABA_SIDE:
            if (building_monument_is_finished(b)) {
               building *main = b->main();
               int image_id = building_small_mastabe_get_bricks_image(b->data.monuments.orientation, b->type, b->tile, main->tile, main->tile.shifted(3, 9), 6);
               map_building_tiles_add(i, b->tile, b->size, image_id, TERRAIN_BUILDING);
            }
            break;

        case BUILDING_LARGE_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_SMALL_STATUE:
            {
                int image_id = building_statue_get_image_from_value(b->type, 0, b->data.monuments.variant, map_orientation);
                map_building_tiles_add(i, b->tile, b->size, image_id, TERRAIN_BUILDING);
            }
            break;
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
            if (b->is_main()) {
                // first, add the base tiles
                int orientation = (5 - (b->data.monuments.variant / 2)) % 4;
                map_add_temple_complex_base_tiles(b->type, b->tile.x(), b->tile.y(), orientation);
                // then, the main building parts
                map_building_tiles_add_temple_complex_parts(b);
            }
            break;
        }
    }
}

const uint8_t BOOTH_ROAD_POSITIONS[4][2][2] = {
  {
    {0, 1},
    {1, 1},
  },
  {
    {1, 0},
    {1, 1},
  },
  {
    {1, 1},
    {1, 0},
  },
  {
    {1, 1},
    {0, 1},
  },
};
const uint8_t BANDSTAND_ROAD_POSITIONS[4][3][3] = {
  {
    {0, 1, 0},
    {0, 1, 0},
    {1, 1, 1},
  },
  {
    {1, 0, 0},
    {1, 1, 1},
    {1, 0, 0},
  },
  {
    {1, 1, 1},
    {0, 1, 0},
    {0, 1, 0},
  },
  {
    {0, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
  },
};
const uint8_t PAVILLION_ROAD_POSITIONS[4][4][4] = {
  {
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {1, 1, 1, 1},
  },
  {
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 1, 1, 1},
    {1, 0, 0, 0},
  },
  {
    {1, 1, 1, 1},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
  },
  {
    {0, 0, 0, 1},
    {1, 1, 1, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
  },
};
const uint8_t FESTIVAL_ROAD_POSITIONS[5][5] = {
  {0, 0, 1, 0, 0},
  {0, 0, 1, 0, 0},
  {1, 1, 1, 1, 1},
  {0, 0, 1, 0, 0},
  {0, 0, 1, 0, 0},
};

bool map_orientation_for_venue(int x, int y, int mode, int* building_orientation) {
    *building_orientation = 0;
    int num_correct_road_tiles[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    int grid_offset = MAP_OFFSET(x, y);
    for (int y_delta = 0; y_delta < mode + 2; y_delta++)
        for (int x_delta = 0; x_delta < mode + 2; x_delta++) {
            int offset_check = grid_offset + GRID_OFFSET(x_delta, y_delta);
            int is_road = map_terrain_is(offset_check, TERRAIN_ROAD);
            if (map_terrain_is(offset_check, TERRAIN_BUILDING)
                || (map_terrain_is(offset_check, TERRAIN_NOT_CLEAR) && !is_road))
                return false;
            else {
                for (int orientation_check = 0; orientation_check < 8; orientation_check++) {
                    int tile_road_checked_against = 0;
                    int half_orientation_check = orientation_check / 2;
                    switch (mode) {
                    case 0: // only 4 orientations
                        tile_road_checked_against = BOOTH_ROAD_POSITIONS[half_orientation_check][y_delta][x_delta];
                        break;
                    case 1: // only 4 orientations
                        tile_road_checked_against = BANDSTAND_ROAD_POSITIONS[half_orientation_check][y_delta][x_delta];
                        break;
                    case 2: // ugh...
                        switch (orientation_check) {
                        case 0:
                            tile_road_checked_against = PAVILLION_ROAD_POSITIONS[0][y_delta][x_delta];
                            break;
                        case 1:
                            tile_road_checked_against = PAVILLION_ROAD_POSITIONS[0][y_delta][3 - x_delta];
                            break;
                        case 2:
                            tile_road_checked_against = PAVILLION_ROAD_POSITIONS[1][y_delta][x_delta];
                            break;
                        case 3:
                            tile_road_checked_against = PAVILLION_ROAD_POSITIONS[1][3 - y_delta][x_delta];
                            break;
                        case 4:
                            tile_road_checked_against = PAVILLION_ROAD_POSITIONS[2][y_delta][x_delta];
                            break;
                        case 5:
                            tile_road_checked_against = PAVILLION_ROAD_POSITIONS[2][y_delta][3 - x_delta];
                            break;
                        case 6:
                            tile_road_checked_against = PAVILLION_ROAD_POSITIONS[3][y_delta][x_delta];
                            break;
                        case 7:
                            tile_road_checked_against = PAVILLION_ROAD_POSITIONS[3][3 - y_delta][x_delta];
                            break;
                        }
                        break;
                    case 3: // only one orientation
                        orientation_check = 7;
                        tile_road_checked_against = FESTIVAL_ROAD_POSITIONS[y_delta][x_delta];
                        break;
                    }

                    if (tile_road_checked_against == is_road)
                        num_correct_road_tiles[orientation_check]++;
                }
            }
        }

    // check the final count and return orientation
    for (int orientation_check = 0; orientation_check < 8; orientation_check++)
        if (num_correct_road_tiles[orientation_check] == (mode + 2) * (mode + 2)) { // check if the num of correct tiles is ALL of them (n x n)
            if (mode == 0) {
                int offset_1 = 0;
                int offset_2 = 0;
                switch (orientation_check) {
                case 0:
                    offset_1 = grid_offset + GRID_OFFSET(1, 2);
                    offset_2 = grid_offset + GRID_OFFSET(2, 1);
                    break;
                case 2:
                    offset_1 = grid_offset + GRID_OFFSET(-1, 1);
                    offset_2 = grid_offset + GRID_OFFSET(0, 2);
                    break;
                case 4:
                    offset_1 = grid_offset + GRID_OFFSET(0, -1);
                    offset_2 = grid_offset + GRID_OFFSET(-1, 0);
                    break;
                case 6:
                    offset_1 = grid_offset + GRID_OFFSET(1, -1);
                    offset_2 = grid_offset + GRID_OFFSET(2, 0);
                    break;
                }
                if (map_terrain_is(offset_1, TERRAIN_ROAD) || map_terrain_is(offset_2, TERRAIN_ROAD)) {
                    *building_orientation = orientation_check;
                    return true;
                }
            } else {
                *building_orientation = orientation_check;
                return true;
            }
        }
    return false;
}
bool map_orientation_for_venue_with_map_orientation(int x, int y, int mode, int* building_orientation) {
    int map_orientation = city_view_orientation();
    switch (map_orientation) {
    case 2: // east
        x -= (mode + 1);
        break;
    case 4: // south
        x -= (mode + 1);
        y -= (mode + 1);
        break;
    case 6: // west
        y -= (mode + 1);
        break;
    }
    return map_orientation_for_venue(x, y, mode, building_orientation);
}
int map_orientation_for_gatehouse(int x, int y) {
    switch (city_view_orientation()) {
    case DIR_2_BOTTOM_RIGHT:
        x--;
        break;
    case DIR_4_BOTTOM_LEFT:
        x--;
        y--;
        break;
    case DIR_6_TOP_LEFT:
        y--;
        break;
    }
    int grid_offset = MAP_OFFSET(x, y);
    int num_road_tiles_within = 0;
    int road_tiles_within_flags = 0;
    // tiles within gate, flags:
    // 1  2
    // 4  8
    if (map_terrain_is(MAP_OFFSET(x, y), TERRAIN_ROAD)) {
        road_tiles_within_flags |= 1;
        num_road_tiles_within++;
    }
    if (map_terrain_is(grid_offset + GRID_OFFSET(1, 0), TERRAIN_ROAD)) {
        road_tiles_within_flags |= 2;
        num_road_tiles_within++;
    }
    if (map_terrain_is(grid_offset + GRID_OFFSET(0, 1), TERRAIN_ROAD)) {
        road_tiles_within_flags |= 4;
        num_road_tiles_within++;
    }
    if (map_terrain_is(grid_offset + GRID_OFFSET(1, 1), TERRAIN_ROAD)) {
        road_tiles_within_flags |= 8;
        num_road_tiles_within++;
    }

    if (num_road_tiles_within != 2 && num_road_tiles_within != 4)
        return 0;

    if (num_road_tiles_within == 2) {
        if (road_tiles_within_flags == 6 || road_tiles_within_flags == 9) { // diagonals
            return 0;
        }
        if (road_tiles_within_flags == 5 || road_tiles_within_flags == 10) { // top to bottom
            return 1;
        }
        if (road_tiles_within_flags == 3 || road_tiles_within_flags == 12) { // left to right
            return 2;
        }
        return 0;
    }
    // all 4 tiles are road: check adjacent roads
    int num_road_tiles_top = 0;
    int num_road_tiles_right = 0;
    int num_road_tiles_bottom = 0;
    int num_road_tiles_left = 0;
    // top
    if (map_terrain_is(grid_offset + GRID_OFFSET(0, -1), TERRAIN_ROAD))
        num_road_tiles_top++;

    if (map_terrain_is(grid_offset + GRID_OFFSET(1, -1), TERRAIN_ROAD))
        num_road_tiles_top++;

    // bottom
    if (map_terrain_is(grid_offset + GRID_OFFSET(0, 2), TERRAIN_ROAD))
        num_road_tiles_bottom++;

    if (map_terrain_is(grid_offset + GRID_OFFSET(1, 2), TERRAIN_ROAD))
        num_road_tiles_bottom++;

    // left
    if (map_terrain_is(grid_offset + GRID_OFFSET(-1, 0), TERRAIN_ROAD))
        num_road_tiles_left++;

    if (map_terrain_is(grid_offset + GRID_OFFSET(-1, 1), TERRAIN_ROAD))
        num_road_tiles_left++;

    // right
    if (map_terrain_is(grid_offset + GRID_OFFSET(2, 0), TERRAIN_ROAD))
        num_road_tiles_right++;

    if (map_terrain_is(grid_offset + GRID_OFFSET(2, 1), TERRAIN_ROAD))
        num_road_tiles_right++;

    // determine direction
    if (num_road_tiles_top || num_road_tiles_bottom) {
        if (num_road_tiles_left || num_road_tiles_right)
            return 0;

        return 1;
    } else if (num_road_tiles_left || num_road_tiles_right)
        return 2;

    return 0;
}
int map_orientation_for_triumphal_arch(int x, int y) {
    switch (city_view_orientation()) {
    case DIR_2_BOTTOM_RIGHT:
        x -= 2;
        break;
    case DIR_4_BOTTOM_LEFT:
        x -= 2;
        y -= 2;
        break;
    case DIR_6_TOP_LEFT:
        y -= 2;
        break;
    }
    int num_road_tiles_top_bottom = 0;
    int num_road_tiles_left_right = 0;
    int num_blocked_tiles = 0;

    int grid_offset = MAP_OFFSET(x, y);
    // check corner tiles
    if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    if (map_terrain_is(grid_offset + GRID_OFFSET(2, 0), TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    if (map_terrain_is(grid_offset + GRID_OFFSET(0, 2), TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    if (map_terrain_is(grid_offset + GRID_OFFSET(2, 2), TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    // road tiles top to bottom
    int top_offset = grid_offset + GRID_OFFSET(1, 0);
    if ((map_terrain_get(top_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD)
        num_road_tiles_top_bottom++;
    else if (map_terrain_is(top_offset, TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    int bottom_offset = grid_offset + GRID_OFFSET(1, 2);
    if ((map_terrain_get(bottom_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD)
        num_road_tiles_top_bottom++;
    else if (map_terrain_is(bottom_offset, TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    // road tiles left to right
    int left_offset = grid_offset + GRID_OFFSET(0, 1);
    if ((map_terrain_get(left_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD)
        num_road_tiles_left_right++;
    else if (map_terrain_is(left_offset, TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    int right_offset = grid_offset + GRID_OFFSET(2, 1);
    if ((map_terrain_get(right_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD)
        num_road_tiles_left_right++;
    else if (map_terrain_is(right_offset, TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    // center tile
    int center_offset = grid_offset + GRID_OFFSET(2, 1);
    if ((map_terrain_get(center_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
        // do nothing
    } else if (map_terrain_is(center_offset, TERRAIN_NOT_CLEAR))
        num_blocked_tiles++;

    // judgement time
    if (num_blocked_tiles)
        return 0;

    if (!num_road_tiles_left_right && !num_road_tiles_top_bottom)
        return 0; // no road: can't determine direction

    if (num_road_tiles_top_bottom == 2 && !num_road_tiles_left_right)
        return 1;

    if (num_road_tiles_left_right == 2 && !num_road_tiles_top_bottom)
        return 2;

    return 0;
}
