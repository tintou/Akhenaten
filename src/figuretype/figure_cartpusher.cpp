#include "figure_cartpusher.h"

#include "building/building_barracks.h"
#include "building/building_granary.h"
#include "building/industry.h"
#include "building/storage.h"
#include "building/building_type.h"
#include "building/building_storage_yard.h"
#include "building/building_storage_room.h"
#include "building/monuments.h"
#include "city/buildings.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/profiler.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "grid/road_network.h"
#include "grid/routing/routing_terrain.h"
#include "config/config.h"

#include "city/finance.h"
#include "js/js_game.h"

struct cartpusher_model : public figures::model_t<FIGURE_CART_PUSHER, figure_cartpusher> {};
cartpusher_model cartpusher_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_figure_cartpusher);
void config_load_figure_cartpusher() {
    g_config_arch.r_section("figure_cartpusher", [] (archive arch) {
        cartpusher_m.anim.load(arch);
    });
}

static const int CART_OFFSET_MULTIPLE_LOADS_FOOD[] = {0, 0, 8, 16, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const int CART_OFFSET_MULTIPLE_LOADS_NON_FOOD[] = {0, 0, 0, 0, 0, 8, 0, 16, 24, 32, 40, 48, 56, 64, 72, 80};
static const int CART_OFFSET_8_LOADS_FOOD[] = {0, 40, 48, 56, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void figure_carrier::load_resource(e_resource resource, int amount) {
    base.resource_id = resource;
    base.resource_amount_full = amount;
}

int figure_carrier::dump_resource(int amount) {
    amount = std::min<int>(amount, base.resource_amount_full);
    base.resource_amount_full -= amount;

    // automatically clear field if carrying nothing
    if (base.resource_amount_full == 0)
        base.resource_id = RESOURCE_NONE;

    return base.resource_amount_full;
}

int figure::get_carrying_amount() {
    return resource_amount_full;
}

void figure_cartpusher::do_deliver(bool warehouseman, int action_done) {
    base.anim_frame = 0;
    base.wait_ticks++;

    if ((!warehouseman && base.wait_ticks >= 10) || (warehouseman && base.wait_ticks >= 4)) {
        base.wait_ticks = 0;

        int carrying = base.get_carrying_amount();
        e_resource resource = base.get_resource();

        // carrying nothing? done!
        if (resource == RESOURCE_NONE || carrying <= 0) {
            base.progress_inside_speed = 0;
            return advance_action(action_done);

        } else {
            building* dest = destination();

            int accepting = 0;
            auto warehouse = storage_yard_cast(dest);
            if (warehouse) {
                accepting = warehouse->accepting_amount(base.resource_id);
            }

            if (!accepting) {
                accepting = 200 - dest->stored_amount(resource);
            }

            int total_depositable = std::min<int>(carrying, accepting);
            if (total_depositable <= 0) {
                return advance_action(ACTION_8_RECALCULATE);
            }

            int max_single_turn = 100;
            if (true) // TODO: more than 100 at once?????
                max_single_turn = total_depositable;

            int amount_single_turn = fmin(total_depositable, max_single_turn);
            int times = total_depositable / amount_single_turn;

            switch (dest->type) {
            case BUILDING_GRANARY:
                {
                    building_granary *granary = dest->dcast_granary();
                    int amount = granary->add_resource(base.resource_id, 0, amount_single_turn);
                    if (amount != -1) {
                        dump_resource(amount_single_turn);
                    } else {
                        return advance_action(ACTION_8_RECALCULATE);
                    }
                }
                break;

            case BUILDING_RECRUITER:
                for (int i = 0; i < times; i++) { // do one by one...
                    dest->barracks_add_weapon(amount_single_turn);
                    dump_resource(amount_single_turn); // assume barracks will ALWAYS accept a weapon
                }
                break;

            case BUILDING_SCRIBAL_SCHOOL:
                for (int i = 0; i < times; i++) { // do one by one...
                    dest->school_add_papyrus(amount_single_turn);
                    dump_resource(amount_single_turn);
                }
                break;

            case BUILDING_STORAGE_YARD:
            case BUILDING_STORAGE_ROOM:
                for (int i = 0; i < times; i++) { // do one by one...
                    auto warehouse = storage_yard_cast(dest);
                    int amount_refused = warehouse->add_resource(base.resource_id, amount_single_turn);
                    if (amount_refused != -1) {
                        dump_resource(amount_single_turn - amount_refused);
                    } else {
                        return advance_action(ACTION_8_RECALCULATE);
                    }
                }
                break;

            case BUILDING_VILLAGE_PALACE:
            case BUILDING_TOWN_PALACE:
            case BUILDING_CITY_PALACE:
                city_finance_process_gold_extraction(amount_single_turn, &base);
                dump_resource(amount_single_turn);
                break;

            default:                              // workshop
                for (int i = 0; i < times; i++) { // do one by one...
                    if (dest->stored_amount(resource) < 200) {
                        building_workshop_add_raw_material(dest, 100, resource);
                        dump_resource(100);
                        if (i + 1 == times) {
                            advance_action(action_done);
                        }
                    } else {
                        return advance_action(ACTION_8_RECALCULATE);
                    }
                }
                advance_action(ACTION_8_RECALCULATE);
                break;
            }
        }

        // am I done?
        if (resource == RESOURCE_NONE || carrying <= 0) {
            return advance_action(action_done);
        }
    }
}

void figure_cartpusher::do_retrieve(int action_done) {
    base.wait_ticks++;
    base.anim_frame = 0;
    if (base.wait_ticks > 4) {
        building* dest = destination();
        switch (dest->type) {
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_ROOM: {
            int home_accepting_quantity= get_storage_accepting_amount(home(), (e_resource)base.collecting_item_id);
            int carry_amount_goal_max = fmin(100, home_accepting_quantity);
            int load_single_turn = 1;
            building_storage_yard *warehouse = dest->dcast_storage_yard();
            if (!warehouse) {
                building_storage_room *room = dest->dcast_storage_room();
                warehouse = room->yard();
            }

            if (true) // TODO: multiple loads setting?????
                carry_amount_goal_max = fmin(400, home_accepting_quantity);

            if (true) // TODO: more than 100 at once?????
                load_single_turn = 4;

            // grab goods, quantity & max load changed by above settings;
            // if load is finished, go back home - otherwise, recalculate
            if (base.get_carrying_amount() < carry_amount_goal_max) {
                if (warehouse->remove_resource((e_resource)base.collecting_item_id, load_single_turn) == 0) {
                    load_resource((e_resource)base.collecting_item_id, load_single_turn * 100);
                    if (base.get_carrying_amount() >= carry_amount_goal_max) {
                        advance_action(action_done);
                    }
                } else {
                    advance_action(ACTION_8_RECALCULATE);
                }
            }
            break;
        }
        case BUILDING_GRANARY: {
            e_resource resource;
            int loads = building_granary::remove_for_getting_deliveryman(destination(), home(), resource);
            load_resource(resource, loads * 100);
            advance_action(FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD);
            break;
        }
        }
    }
}

void figure_cartpusher::calculate_destination(bool warehouseman) {
    set_destination(0);
    base.anim_frame = 0;
    base.wait_ticks++;

    if (!warehouseman) {
        if (base.wait_ticks > 30) {
            base.wait_ticks = 0;
            determine_deliveryman_destination();
        }
    } else {
        building* b = home();
        if (base.wait_ticks > 2) {
            base.wait_ticks = 0;
            if (home()->type == BUILDING_GRANARY) {
                determine_granaryman_destination();
            } else {
                determine_storageyard_cart_destination();
            }
        }
    }
}

void figure_cartpusher::determine_deliveryman_destination() {
    tile2i dst;
    int road_network_id = map_road_network_get(tile());
    building* warehouse = home();
    int understaffed_storages = 0;

    destination_tile.set(0, 0);
    //    destination_tile.x() = 0;
    //    destination_tile.y() = 0;

    // before we start... check that resource is not empty.
    if (base.resource_id == RESOURCE_NONE || base.get_carrying_amount() == 0) {
        return advance_action(ACTION_15_RETURNING2);
    }

    // first: gold deliverers
    if (base.resource_id == RESOURCE_GOLD) {
        int senate_id = city_buildings_get_palace_id();
        building* b = building_get(senate_id);
        if (senate_id && b->state == BUILDING_STATE_VALID && b->num_workers >= 5) {
            set_destination(senate_id);
            return advance_action(ACTION_11_DELIVERING_GOLD);
        }
    }

    // priority 1: warehouse if resource is on stockpile
    int stockpile_id = building_storage_yard_for_storing(tile(), base.resource_id, warehouse->distance_from_entry, road_network_id, &understaffed_storages, &dst);
    set_destination(stockpile_id);
    if (!city_resource_is_stockpiled(base.resource_id)) {
        set_destination(0);
    }

    if (has_destination()) {
        return advance_action(FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE);
    }

    // priority 2: accepting granary for food
    int granary_food_id = building_granary_for_storing(tile(), base.resource_id, warehouse->distance_from_entry, road_network_id, 0, &understaffed_storages, &dst);
    set_destination(granary_food_id);
    if (config_get(CONFIG_GP_CH_FARMS_DELIVER_CLOSE)) {
        int dist = 0;
        building* src_building = home();
        building* dst_building = destination();
        int src_int = src_building->type;
        if ((src_int >= BUILDING_BARLEY_FARM && src_int <= BUILDING_CHICKPEAS_FARM) || src_int == BUILDING_FISHING_WHARF) {
            dist = calc_distance_with_penalty(src_building->tile, dst_building->tile, src_building->distance_from_entry,dst_building->distance_from_entry);
        }

        if (dist >= 64) {
            set_destination(0);
        }
    }

    if (has_destination()) {
        return advance_action(FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY);
    }

    // priority 3: workshop for raw material
    int workshop_id = building_get_workshop_for_raw_material_with_room(tile(), (e_resource)base.resource_id, warehouse->distance_from_entry, road_network_id, dst);
    set_destination(workshop_id);
    if (has_destination()) {
        return advance_action(FIGURE_ACTION_23_CARTPUSHER_DELIVERING_TO_WORKSHOP);
    }

    // priority 4: warehouse
    int warehouse_id = building_storage_yard_for_storing(tile(), base.resource_id, warehouse->distance_from_entry, road_network_id, &understaffed_storages, &dst);
    set_destination(warehouse_id);
    if (has_destination()) {
        return advance_action(FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE);
    }

    // priority 5: granary forced when on stockpile
    int granary_id = building_granary_for_storing(tile(), base.resource_id, warehouse->distance_from_entry, road_network_id, 1, &understaffed_storages, &dst);
    set_destination(granary_id);
    if (config_get(CONFIG_GP_CH_FARMS_DELIVER_CLOSE)) {
        int dist = 0;
        building* src_building = home();
        building* dst_building = destination();
        int src_int = src_building->type;
        if ((src_int >= BUILDING_BARLEY_FARM && src_int <= BUILDING_CHICKPEAS_FARM) || src_int == BUILDING_FISHING_WHARF) {
            dist = calc_distance_with_penalty(src_building->tile, dst_building->tile, src_building->distance_from_entry, dst_building->distance_from_entry);
        }

        if (dist >= 64) {
            set_destination(0);
        }
    }

    if (has_destination()) {
        return advance_action(FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY);
    }

    // no one will accept
    base.min_max_seen = understaffed_storages ? 2 : 1;
    advance_action(ACTION_8_RECALCULATE);
}

void figure_cartpusher::determine_granaryman_destination() {
    tile2i dst;
    int road_network_id = map_road_network_get(tile());
    building_granary* granary = home()->dcast_granary();
    if (!base.resource_id) {
        // getting granaryman
        int dest_id = granary->for_getting(&dst);
        set_destination(dest_id);
        if (has_destination()) {
            advance_action(FIGURE_ACTION_54_WAREHOUSEMAN_GETTING_FOOD);
            //            set_destination(FIGURE_ACTION_54_WAREHOUSEMAN_GETTING_FOOD, dst_building_id, dst.x, dst.y);
            if (config_get(CONFIG_GP_CH_GETTING_GRANARIES_GO_OFFROAD)) {
                base.terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
            }
        } else {
            poof();
        }
        return;
    }
    // delivering resource
    // priority 1: another granary
    set_destination(building_granary_for_storing(tile(), base.resource_id, granary->distance_from_entry(), road_network_id, 0, 0, &dst));
    if (has_destination()) {
        granary->remove_resource(base.resource_id, 100);
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    }

    // priority 2: warehouse
    int warehouse_id = building_storage_yard_for_storing(tile(), base.resource_id, granary->distance_from_entry(), road_network_id, 0, &dst);
    set_destination(warehouse_id);
    if (has_destination()) {
        granary->remove_resource(base.resource_id, 100);
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    }

    // priority 3: granary even though resource is on stockpile
    int granary_second_id = building_granary_for_storing(tile(), base.resource_id, granary->distance_from_entry(), road_network_id, 1, 0, &dst);
    set_destination(granary_second_id);
    if (has_destination()) {
        granary->remove_resource(base.resource_id, 100);
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    }
    // nowhere to go to: recalculate
    advance_action(FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD);
}

void figure_cartpusher::determine_storageyard_cart_destination() {
    int road_network_id = map_road_network_get(tile());

    ////// getting resources!
    if (!base.resource_id) {
        tile2i dst;
        building_storage_yard *warhouse = home()->dcast_storage_yard();
        int building_id = warhouse
                            ? warhouse->for_getting((e_resource)base.collecting_item_id, &dst)
                            : 0;
        set_destination(building_id);
        if (has_destination()) {
            advance_action(FIGURE_ACTION_57_WAREHOUSEMAN_GETTING_RESOURCE);
            base.terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
        } else {
            advance_action(FIGURE_ACTION_53_WAREHOUSEMAN_RETURNING_EMPTY);
        }
        return;
    }
    building* warehouse = home();

    ////// delivering resource!
    // priority 1: weapons to barracks
    if (base.resource_id == RESOURCE_WEAPONS) {
        auto result = building_get_asker_for_resource(tile(), BUILDING_RECRUITER, base.resource_id, road_network_id, warehouse->distance_from_entry);
        set_destination(result.building_id);
        if (has_destination()) {
            return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
        }
    }

    // priority 2: paryrus to scribal school
    if (base.resource_id == RESOURCE_PAPYRUS) {
        auto result = building_get_asker_for_resource(tile(), BUILDING_SCRIBAL_SCHOOL, base.resource_id, road_network_id, warehouse->distance_from_entry);
        set_destination(result.building_id);
        if (has_destination()) {
            return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
        }
    }

    // priority 3: raw materials to workshop
    {
        tile2i dest;
        int building_id = building_get_workshop_for_raw_material_with_room(tile(), base.resource_id, warehouse->distance_from_entry, road_network_id, dest);
        set_destination(building_id);
        if (has_destination()) {
            return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
        }
    }

    // priority 4: food to granary
    {
        tile2i dest;
        int building_id = building_granary_for_storing(tile(), base.resource_id, warehouse->distance_from_entry, road_network_id, 0, 0, &dest);
        set_destination(building_id);
        if (has_destination()) {
            return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
        }
    }

    // priority 5: food to getting granary
    {
        tile2i dest;
        int building_id = building_getting_granary_for_storing(tile(), base.resource_id, warehouse->distance_from_entry, road_network_id, &dest);
        set_destination(building_id);
        if (has_destination()) {
            return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
        }
    }

    // priority 6: resource to other warehouse
    tile2i dest;
    int warehouse_id = building_storage_yard_for_storing(tile(), base.resource_id, warehouse->distance_from_entry, road_network_id, 0, &dest);
    set_destination(warehouse_id);
    if (has_destination()) {
        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    }
    //    int empty_warehouse = building_storage_get(home()->storage_id)->empty_all; // deliver to another warehouse
    //    because this one is being emptied if (has_destination() && empty_warehouse) {
    //        if (homeID() == destinationID())
    //            poof();
    //        else
    //            advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    //        return;
    //    }

    //    // priority 6: raw material to well-stocked workshop
    //    set_destination(building_get_workshop_for_raw_material(tile_x, tile_y, resource_id,
    //    warehouse->distance_from_entry, road_network_id, &dst); if (destination_building_id) {
    //        remove_resource_from_warehouse();
    //        return advance_action(FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE);
    //    }

    // no destination: recalculate
    //    advance_action(FIGURE_ACTION_59_WAREHOUSEMAN_RETURNING_WITH_RESOURCE);
}

void figure::sled_puller_action() {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Figure/SledPuller");
    if (leading_figure_id > 0) {
        --wait_ticks;
        if (wait_ticks > 0) {
            return;
        }

        figure* leader = figure_get(leading_figure_id);
        if (leader->type == FIGURE_SLED_PULLER && leader->state == FIGURE_STATE_ALIVE) {
            follow_ticks(1);
        } else {
            poof();
            return;
        }
    }

    switch (action_state) {
    case ACTION_8_RECALCULATE:
    case FIGURE_ACTION_50_SLED_PULLER_CREATED:
        --wait_ticks;
        if (wait_ticks > 0) {
            return;
        }
        advance_action(FIGURE_ACTION_51_SLED_PULLER_DELIVERING_RESOURCE);
        destination_tile = building_monument_center_point(destination());
        break;

    case FIGURE_ACTION_51_SLED_PULLER_DELIVERING_RESOURCE:
        do_goto(destination_tile, TERRAIN_USAGE_PREFER_ROADS, FIGURE_ACTION_52_SLED_PULLER_AT_DELIVERY_BUILDING, FIGURE_ACTION_53_SLED_PULLER_DESTROY);
        break;

    case FIGURE_ACTION_52_SLED_PULLER_AT_DELIVERY_BUILDING:
        //cartpusher_do_deliver(true, ACTION_11_RETURNING_EMPTY);
        wait_ticks = 25;
        advance_action(FIGURE_ACTION_54_SLED_PULLER_WAITING_FOR_DESTROY);
        break;

    case FIGURE_ACTION_54_SLED_PULLER_WAITING_FOR_DESTROY:
        --wait_ticks;
        if (wait_ticks > 0) {
            return;
        }
        advance_action(FIGURE_ACTION_53_SLED_PULLER_DESTROY);
        break;

    case FIGURE_ACTION_53_SLED_PULLER_DESTROY:
        poof();
        break;
    }
}

void figure_cartpusher::figure_before_action() {
    if (has_destination()) {
        return;
    }

    building* b = home();
    if (!building_is_floodplain_farm(*b)
        && (b->state != BUILDING_STATE_VALID || (!b->has_figure(0, id()) && !b->has_figure(1, id())))) {
        poof();
    }
}

void figure_cartpusher::figure_action() {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Figure/Cartpusher");
    building* b = home();
    int road_network_id = map_road_network_get(tile());
    switch (action_state()) {
    case ACTION_8_RECALCULATE:
    case FIGURE_ACTION_20_CARTPUSHER_INITIAL:
        calculate_destination(false);
        break;
    case ACTION_9_DELIVERING_GOODS:
        do_gotobuilding(destination(), true, TERRAIN_USAGE_ROADS, ACTION_12_UNLOADING1, ACTION_8_RECALCULATE);
        break;
    case ACTION_10_DELIVERING_FOOD:
        do_gotobuilding(destination(), true, TERRAIN_USAGE_ROADS, ACTION_13_UNLOADING2, ACTION_8_RECALCULATE);
        break;
    case ACTION_11_DELIVERING_GOLD:
        do_gotobuilding(destination(), true, TERRAIN_USAGE_ROADS, ACTION_14_UNLOADING_GOLD, ACTION_8_RECALCULATE);
        break;
    case FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE:
        do_gotobuilding(destination(), true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_24_CARTPUSHER_AT_WAREHOUSE, ACTION_8_RECALCULATE);
        break;
    case FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY:
        do_gotobuilding(destination(), true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_25_CARTPUSHER_AT_GRANARY, ACTION_8_RECALCULATE);
        break;
    case FIGURE_ACTION_23_CARTPUSHER_DELIVERING_TO_WORKSHOP:
        do_gotobuilding(destination(), true, TERRAIN_USAGE_ROADS, FIGURE_ACTION_26_CARTPUSHER_AT_WORKSHOP, ACTION_8_RECALCULATE);
        break;
    case 12: // storage yard
    case 13: // granary
    case 14: // palace
    case FIGURE_ACTION_24_CARTPUSHER_AT_WAREHOUSE:
    case FIGURE_ACTION_25_CARTPUSHER_AT_GRANARY:
    case FIGURE_ACTION_26_CARTPUSHER_AT_WORKSHOP:
        do_deliver(false, ACTION_15_RETURNING2);
        break;
    case ACTION_15_RETURNING2:
    case FIGURE_ACTION_27_CARTPUSHER_RETURNING:
        // the CARTPUSHER figure will never be retrieving goods to carry back.
        // that's job for the WAREHOUSEMAN figure!
        // so there is no need for `cartpusher_do_deliver` action.
        if (building_is_floodplain_farm(*b)) { // do not return to floodplain farms
            poof();
        } else {
            do_returnhome(TERRAIN_USAGE_ROADS);
        }
        break;
    }
}

void figure_cartpusher::figure_draw(painter &ctx, vec2i pixel, int highlight, vec2i *coord_out) {
    base.draw_figure_with_cart(ctx, base.cached_pos, highlight, coord_out);
}

sound_key figure_cartpusher::phrase_key() const {
    if (action_state() == FIGURE_ACTION_20_CARTPUSHER_INITIAL) {
        return "cartpusher_i_have_no_destination";
    }

    if (action_state() == FIGURE_ACTION_27_CARTPUSHER_RETURNING) {
        return "cartpusher_back_to_home";
    }

    if (action_state() == FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE ||
        action_state() == FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY ||
        action_state() == FIGURE_ACTION_23_CARTPUSHER_DELIVERING_TO_WORKSHOP) {
        return "cartpusher_delivering_items";
    }

    return {};
}
