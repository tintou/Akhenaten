#include "window/city.h"
#include "industry.h"

#include "city/resource.h"
#include "core/calc.h"
#include "core/image.h"
#include "game/resource.h"
#include "map/building_tiles.h"
#include "map/road_access.h"
#include "scenario/property.h"

#define MAX_PROGRESS_RAW 200
#define MAX_PROGRESS_WORKSHOP 400
#define MAX_PROGRESS_FARM_PH 2000
#define INFINITE 10000

#include "map/terrain.h"
#include "city/view.h"

#include <math.h>
#include <city/floods.h>
#include <map/grid.h>

static int max_progress(const building *b) {
    if (GAME_ENV == ENGINE_ENV_PHARAOH && building_is_farm(b->type))
        return MAX_PROGRESS_FARM_PH;
    return b->subtype.workshop_type ? MAX_PROGRESS_WORKSHOP : MAX_PROGRESS_RAW;
}
static void update_farm_image(const building *b) {
    bool is_flooded = false;
    if (building_is_floodplain_farm(b)) {
        for (int _y = b->y; _y < b->y + b->size; _y++)
            for (int _x = b->x; _x < b->x + b->size; _x++)
                if (map_terrain_is(map_grid_offset(_x, _y), TERRAIN_WATER))
                    is_flooded = true;
    }
    if (!is_flooded)
        map_building_tiles_add_farm(b->id, b->x, b->y,
                                    image_id_from_group(GROUP_BUILDING_FARMLAND) + 5 * (b->output_resource_id - 1),
                                    b->data.industry.progress);
}

int building_determine_worker_needed() {
    for (int i = 1; i < MAX_BUILDINGS[GAME_ENV]; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_VALID)
            continue;
        if (floodplains_is(FLOOD_STATE_FARMABLE)
            && building_is_floodplain_farm(b)
            && !b->data.industry.worker_id
            && b->data.industry.labor_days_left <= 47)
            return i;
        else if (building_is_monument(b->type)) {
            // todo
        }
    }
    return 0; // temp
}

void building_industry_update_production(void) {
    for (int i = 1; i < MAX_BUILDINGS[GAME_ENV]; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_VALID || !b->output_resource_id)
            continue;
        b->data.industry.has_raw_materials = 0;
        if (b->labor_category != 255 && (b->houses_covered <= 0 || b->num_workers <= 0))
            continue;
        if (building_is_floodplain_farm(b)) {
            if (b->data.industry.labor_days_left <= 0 || floodplains_is(FLOOD_STATE_IMMINENT))
                continue;
            else
                int a = 2;
        }
        if (b->subtype.workshop_type && !b->stored_full_amount)
            continue;
        if (b->data.industry.curse_days_left)
            b->data.industry.curse_days_left--;
        else {
            if (b->data.industry.blessing_days_left)
                b->data.industry.blessing_days_left--;

            if (b->type == BUILDING_STONE_QUARRY)
                b->data.industry.progress += b->num_workers / 2;
            else if (building_is_floodplain_farm(b)) {
                int fert = map_get_fertility_average(b->grid_offset);
                b->data.industry.progress += fert * 0.16;
            } else
                b->data.industry.progress += b->num_workers;
            if (b->data.industry.blessing_days_left && building_is_farm(b->type))
                b->data.industry.progress += b->num_workers;

            int max = max_progress(b);
            if (b->data.industry.progress > max)
                b->data.industry.progress = max;

            if (building_is_farm(b->type))
                update_farm_image(b);
        }
    }
}
void building_industry_update_wheat_production(void) {
    if (scenario_property_climate() == CLIMATE_NORTHERN)
        return;
    for (int i = 1; i < MAX_BUILDINGS[GAME_ENV]; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_VALID || !b->output_resource_id)
            continue;
        if (b->houses_covered <= 0 || b->num_workers <= 0)
            continue;
        if (b->type == BUILDING_BARLEY_FARM && !b->data.industry.curse_days_left) {
            b->data.industry.progress += b->num_workers;
            if (b->data.industry.blessing_days_left)
                b->data.industry.progress += b->num_workers;

            if (b->data.industry.progress > MAX_PROGRESS_RAW)
                b->data.industry.progress = MAX_PROGRESS_RAW;

            update_farm_image(b);
        }
    }
}
int building_industry_has_produced_resource(building *b) {
    if (building_is_floodplain_farm(b)) {
        if (floodplains_is(FLOOD_STATE_IMMINENT) && b->data.industry.progress > 0)
            return floodplains_time_to_deliver();
        return 0;
    }
    return b->data.industry.progress >= max_progress(b);
}
void building_industry_start_new_production(building *b) {
    b->data.industry.progress = 0;
    if (b->subtype.workshop_type) {
        if (b->stored_full_amount) {
            if (b->stored_full_amount > 100)
                b->data.industry.has_raw_materials = 1;
            b->stored_full_amount -= 100;
        }
    }
    if (building_is_farm(b->type))
        update_farm_image(b);
}

void building_bless_farms(void) {
    for (int i = 1; i < MAX_BUILDINGS[GAME_ENV]; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->output_resource_id && building_is_farm(b->type)) {
            b->data.industry.progress = MAX_PROGRESS_RAW;
            b->data.industry.curse_days_left = 0;
            b->data.industry.blessing_days_left = 16;
            update_farm_image(b);
        }
    }
}
void building_curse_farms(int big_curse) {
    for (int i = 1; i < MAX_BUILDINGS[GAME_ENV]; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->output_resource_id && building_is_farm(b->type)) {
            b->data.industry.progress = 0;
            b->data.industry.blessing_days_left = 0;
            b->data.industry.curse_days_left = big_curse ? 48 : 4;
            update_farm_image(b);
        }
    }
}
void building_farm_deplete_soil(const building *b) {
    // DIFFERENT from original Pharaoh... and a bit easier to do?
    int malus = (float)b->data.industry.progress / (float)MAX_PROGRESS_FARM_PH * (float)-100;
    for (int _y = b->y; _y < b->y + b->size; _y++)
        for (int _x = b->x; _x < b->x + b->size; _x++)
            map_soil_depletion(map_grid_offset(_x, _y), malus);
    update_farm_image(b);
}

void building_workshop_add_raw_material(building *b, int amount) {
    if (b->id > 0 && building_is_workshop(b->type))
        b->stored_full_amount += amount; // BUG: any raw material accepted
}
int building_get_workshop_for_raw_material_with_room(int x, int y, int resource, int distance_from_entry, int road_network_id, map_point *dst) {
    if (city_resource_is_stockpiled(resource))
        return 0;

    int output_type = resource_to_workshop_type(resource);
    if (output_type == WORKSHOP_NONE)
        return 0;

    int min_dist = INFINITE;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS[GAME_ENV]; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_VALID || !building_is_workshop(b->type))
            continue;

        if (!b->has_road_access || b->distance_from_entry <= 0)
            continue;

        if (b->subtype.workshop_type == output_type && b->road_network_id == road_network_id && b->stored_full_amount < 200) {
            int dist = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
            if (b->stored_full_amount > 0)
                dist += 20;

            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
        return min_building->id;
    }
    return 0;
}
int building_get_workshop_for_raw_material(int x, int y, int resource, int distance_from_entry, int road_network_id, map_point *dst) {
    if (city_resource_is_stockpiled(resource))
        return 0;

    int output_type = resource_to_workshop_type(resource);
    if (output_type == WORKSHOP_NONE)
        return 0;

    int min_dist = INFINITE;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS[GAME_ENV]; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_VALID || !building_is_workshop(b->type))
            continue;

        if (!b->has_road_access || b->distance_from_entry <= 0)
            continue;

        if (b->subtype.workshop_type == output_type && b->road_network_id == road_network_id) {
            int dist = 10 * (b->stored_full_amount / 100) +
                       calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
        return min_building->id;
    }
    return 0;
}
