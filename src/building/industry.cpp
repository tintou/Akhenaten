#include "industry.h"

#include "building/building_type.h"
#include "building/building_farm.h"
#include "building/monuments.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/profiler.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "grid/building_tiles.h"
#include "grid/road_access.h"
#include "scenario/property.h"
#include "window/city.h"
#include "graphics/image_groups.h"
#include "grid/terrain.h"
#include "config/config.h"
#include "city/data_private.h"
#include "city/floods.h"
#include "game/time.h"
#include "grid/floodplain.h"
#include "grid/grid.h"

#include <cmath>

constexpr short MAX_PROGRESS_RAW = 200;
#define MAX_PROGRESS_WORKSHOP 400
#define INFINITE 10000

static int max_progress(building &b) {
    if (building_is_farm(b.type)) {
        return MAX_PROGRESS_FARM_PH;
    }

    return building_is_workshop(b.type) ? MAX_PROGRESS_WORKSHOP : MAX_PROGRESS_RAW;
}

delivery_destination building_get_asker_for_resource(tile2i tile, e_building_type btype, e_resource resource, int road_network_id, int distance_from_entry) {
    if (city_resource_is_stockpiled(resource)) {
        return {0};
    }

    int min_dist = INFINITE;
    building* min_building = 0;
    buildings_valid_do([&] (building &b) {
        if (b.type != btype) {
            return;
        }

        if (!map_has_road_access(b.tile, b.size)) {
            return;
        }

        if (b.distance_from_entry <= 0 || b.road_network_id != road_network_id) {
            return;
        }

        if (b.stored_full_amount >= b.need_resource_amount(resource) * 100) {
            return;
        }

        int dist = calc_distance_with_penalty(b.tile, tile, distance_from_entry, b.distance_from_entry);
        dist += 8 * b.stored_full_amount / 100;
        if (dist < min_dist) {
            min_dist = dist;
            min_building = &b;
        }
    });

    if (min_building && min_dist < INFINITE) {
        tile2i dst;
        map_point_store_result(min_building->road_access, dst);
        return {min_building->id, dst};
    }

    return {0};
}

building* building_determine_worker_needed() {
    return building_first([] (building &b) {
        if (floodplains_is(FLOOD_STATE_FARMABLE) && building_is_floodplain_farm(b)) {
            return (!b.data.industry.worker_id && b.data.industry.labor_days_left <= 47 && !b.num_workers);
        } else if (building_is_monument(b.type) && b.data.monuments.phase < 2) {
            return building_monument_need_workers(&b);
        }

        return false;
    });
}

static const float produce_uptick_per_day = 103.5f * 20.0f / 128.0f / 100.0f; // don't ask

float get_farm_produce_uptick_per_day(building &b) {
    return produce_uptick_per_day;
}

// void building_bless_farms(void) {
//     for (int i = 1; i < MAX_BUILDINGS; i++) {
//         building *b = building_get(i);
//         if (b->state == BUILDING_STATE_VALID && b->output_resource_id && building_is_farm(b->type)) {
//             b->data.industry.progress = MAX_PROGRESS_RAW;
//             b->data.industry.curse_days_left = 0;
//             b->data.industry.blessing_days_left = 16;
//             update_farm_image(b);
//         }
//     }
// }
int farm_expected_produce(building* b) {
    int progress = b->data.industry.ready_production > 0
                        ? b->data.industry.ready_production
                        : b->data.industry.progress;
    if (!config_get(CONFIG_GP_FIX_FARM_PRODUCE_QUANTITY))
        progress = (progress / 20) * 20;
    // In OG Pharaoh, the progress value gets counted as if it was rounded
    // down to the lowest 20 points. No idea why! But here's as an option.

    int modifier = 1;
    if (city_data.religion.osiris_double_farm_yield && building_is_floodplain_farm(*b)) {
        modifier = 2;
    }

    return int((progress / 2.5f) * modifier);
}

void building_industry_update_production(void) {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Industry Update");
    buildings_valid_do([] (building &b) {
        if (!b.output_resource_first_id) {
            return;
        }

        if (building_is_farm(b.type)) {
            return;
        }

        b.data.industry.has_raw_materials = false;
        if (b.num_workers <= 0) {
            return;
        }

        if (building_is_workshop(b.type) && !b.workshop_has_resources()) {
            return;
        }

        if (b.data.industry.curse_days_left) {
            b.data.industry.curse_days_left--;
        } else {
            if (b.data.industry.blessing_days_left) {
                b.data.industry.blessing_days_left--;
            }

            int progress_per_day = b.dcast()->get_produce_uptick_per_day();
            b.data.industry.progress += progress_per_day;

            if (b.data.industry.blessing_days_left && building_is_farm(b.type)) {
                const float normal_progress = progress_per_day;
                b.data.industry.progress += normal_progress;
            }

            int max = max_progress(b);
            if (b.data.industry.progress > max) {
                b.data.industry.progress = max;
            }
        }
    });
}

void building_industry_update_farms(void) {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Farms Update");

    buildings_valid_farms_do([] (building &b) {
        if (!b.output_resource_first_id) {
            return;
        }

        if (b.data.industry.curse_days_left) { // TODO
            b.data.industry.curse_days_left--;
        }

        if (b.data.industry.blessing_days_left) {
            b.data.industry.blessing_days_left--;
        }
        //        if (b->data.industry.blessing_days_left && building_is_farm(b->type))
        //            b->data.industry.progress += b->num_workers;

        bool is_floodplain = building_is_floodplain_farm(b);
        int fert = map_get_fertility_for_farm(b.tile.grid_offset());
        int progress_step = (float)fert * get_farm_produce_uptick_per_day(b); // 0.16f

        if (is_floodplain) { // floodplain farms
            // advance production
            if (b.data.industry.labor_days_left > 0) {
                b.data.industry.progress += progress_step;
            }
            // update labor state
            if (b.data.industry.labor_state == LABOR_STATE_JUST_ENTERED) {
                b.data.industry.labor_state = LABOR_STATE_PRESENT;
            }

            if (b.data.industry.labor_days_left == 0) {
                b.data.industry.labor_state = LABOR_STATE_NONE;
            }

            if (b.data.industry.labor_days_left > 0) {
                b.data.industry.labor_days_left--;
            }
        } else { // meadow farms
            // advance production
            if (b.num_workers > 0) {
                b.data.industry.progress += progress_step * ((float)b.num_workers / 10.0f);
            }
        }

        // clamp progress
        int max = max_progress(b);
        b.data.industry.progress = std::clamp<int>(b.data.industry.progress, 0, max);

        building_farm *farm = b.dcast_farm();
        if (farm) {
            farm->update_tiles_image();
        }
    });
    city_data.religion.osiris_double_farm_yield = false;
}

void building_industry_update_wheat_production() {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Wheat Production Update");
    if (scenario_property_climate() == CLIMATE_NORTHERN)
        return;

    buildings_valid_do([] (building &b) {
        if (!b.output_resource_first_id) {
            return;
        }

        if (b.houses_covered <= 0 || b.num_workers <= 0) {
            return;
        }

        if (b.type == BUILDING_BARLEY_FARM && !b.data.industry.curse_days_left) {
            b.data.industry.progress += b.num_workers;
            if (b.data.industry.blessing_days_left)
                b.data.industry.progress += b.num_workers;

            b.data.industry.progress = std::min<short>(b.data.industry.progress, MAX_PROGRESS_RAW);

            b.dcast_farm()->update_tiles_image();
        }
    });
}

bool building_industry_has_produced_resource(building &b) {
    return b.data.industry.progress >= max_progress(b);
}

void building_industry_start_new_production(building* b) {
    b->data.industry.progress = 0;
    if (building_is_workshop(b->type)) {
        if (b->workshop_has_resources()) {
            b->workshop_start_production();
        }
    }

    building_farm *farm = b->dcast_farm();
    if (farm) {
        farm->update_tiles_image();
    }
}

void building_curse_farms(int big_curse) {
    // TODO
    //    for (int i = 1; i < MAX_BUILDINGS; i++) {
    //        building *b = building_get(i);
    //        if (b->state == BUILDING_STATE_VALID && b->output_resource_id && building_is_farm(b->type)) {
    //            b->data.industry.progress = 0;
    //            b->data.industry.blessing_days_left = 0;
    //            b->data.industry.curse_days_left = big_curse ? 48 : 4;
    //            update_farm_image(b);
    //        }
    //    }
}

void building_workshop_add_raw_material(building* b, int amount, e_resource res) {
    if (b->id > 0
        && building_is_workshop(b->type)
        && resource_required_by_workshop(b, res)) {
        if (b->data.industry.first_material_id == res) {
            b->stored_full_amount += amount; // BUG: any raw material accepted
        } else if (b->data.industry.second_material_id == res) {
            b->data.industry.stored_amount_second += amount;
        } else {
            assert(false);
        }
    } else {
        //assert(false);
    }
}
int building_get_workshop_for_raw_material_with_room(tile2i tile, e_resource resource, int distance_from_entry, int road_network_id, tile2i &dst) {
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }

    int min_dist = INFINITE;
    building* min_building = 0;
    buildings_valid_do([&] (building &b) {
        if (!building_is_workshop(b.type)) {
            return;
        }

        if (!b.has_road_access || b.distance_from_entry <= 0) {
            return;
        }

        if (resource_required_by_workshop(&b, resource) && b.road_network_id == road_network_id && b.stored_amount(resource) < 200) {
            int dist = calc_distance_with_penalty(b.tile, tile, distance_from_entry, b.distance_from_entry);
            if (b.stored_amount(resource) > 0) {
                dist += 20;
            }

            if (dist < min_dist) {
                min_dist = dist;
                min_building = &b;
            }
        }
    });

    if (min_building) {
        map_point_store_result(min_building->road_access, dst);
        return min_building->id;
    }

    return 0;
}
int building_get_workshop_for_raw_material(tile2i tile, e_resource resource, int distance_from_entry, int road_network_id, tile2i &dst) {
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }

    int min_dist = INFINITE;
    building* min_building = nullptr;
    buildings_valid_do([&] (building &b) {
        if (!building_is_workshop(b.type)) {
            return;
        }

        if (!b.has_road_access || b.distance_from_entry <= 0) {
            return;
        }

        if (resource_required_by_workshop(&b, resource) && b.road_network_id == road_network_id) {
            int dist = 10 * (b.stored_amount(resource) / 100) + calc_distance_with_penalty(b.tile, tile, distance_from_entry, b.distance_from_entry);
            if (dist < min_dist) {
                min_dist = dist;
                min_building = &b;
            }
        }
    });

    if (min_building) {
        map_point_store_result(min_building->road_access, dst);
        return min_building->id;
    }

    return 0;
}