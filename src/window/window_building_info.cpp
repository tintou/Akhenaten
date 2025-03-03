#include "window_building_info.h"

#include "building/building_barracks.h"
#include "building/building.h"
#include "building/building_tax_collector.h"
#include "building/building_palace.h"
#include "building/building_statue.h"
#include "building/house_evolution.h"
#include "building/building_bandstand.h"
#include "building/building_farm.h"
#include "building/building_hunting_lodge.h"
#include "building/building_raw_material.h"
#include "building/building_workshop.h"
#include "building/building_wharf.h"
#include "building/building_shipyard.h"
#include "building/building_architect_post.h"
#include "building/building_shrine.h"
#include "building/building_temple.h"
#include "building/building_booth.h"
#include "building/building_entertainment.h"
#include "building/building_health.h"
#include "building/building_education.h"
#include "building/building_bricklayers_guild.h"
#include "building/model.h"
#include "building/storage.h"
#include "building/building_storage_yard.h"
#include "city/map.h"
#include "city/resource.h"
#include "overlays/city_overlay.h"
#include "core/calc.h"
#include "game/state.h"
#include "figure/figure.h"
#include "figure/formation_legion.h"
#include "figure/figure_phrase.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/elements/ui.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/view/view.h"
#include "graphics/window.h"
#include "grid/aqueduct.h"
#include "grid/building.h"
#include "grid/figure.h"
#include "grid/grid.h"
#include "grid/image.h"
#include "grid/point.h"
#include "grid/property.h"
#include "grid/road_access.h"
#include "grid/sprite.h"
#include "grid/terrain.h"
#include "game/game.h"
#include "input/input.h"
#include "window/advisors.h"
#include "window/building/common.h"
#include "window/building/culture.h"
#include "window/building/distribution.h"
#include "window/building/figures.h"
#include "window/building/government.h"
#include "window/building/house.h"
#include "window/building/military.h"
#include "window/building/terrain.h"
#include "window/building/utility.h"
#include "window/city.h"
#include "window/message_dialog.h"

#include <functional>
#include <utility>

object_info g_building_info_context;

static int get_height_id() {
    auto &context = g_building_info_context;
    if (context.type == BUILDING_INFO_TERRAIN) {
        switch (context.terrain_type) {
        case TERRAIN_INFO_AQUEDUCT:
            return 4;
        case TERRAIN_INFO_RUBBLE:
        case TERRAIN_INFO_WALL:
        case TERRAIN_INFO_GARDEN:
            return 1;
        default:
            return 5;
        }
    } else if (context.type == BUILDING_INFO_BUILDING) {
        const building* b = building_get(context.building_id);
        if (building_is_house(b->type) && b->house_population <= 0)
            return 5;

        switch (b->type) {
        case BUILDING_TEMPLE_OSIRIS:
        case BUILDING_TEMPLE_RA:
        case BUILDING_TEMPLE_PTAH:
        case BUILDING_TEMPLE_SETH:
        case BUILDING_TEMPLE_BAST:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_ORACLE:
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_CONSERVATORY:
        case BUILDING_DANCE_SCHOOL:
        case BUILDING_JUGGLER_SCHOOL:
        case BUILDING_SENET_MASTER:
        case BUILDING_APOTHECARY:
        case BUILDING_MORTUARY:
        case BUILDING_MENU_MONUMENTS:
        case BUILDING_DENTIST:
        case BUILDING_BURNING_RUIN:
        case BUILDING_WATER_LIFT:
        case BUILDING_UNUSED_NATIVE_HUT_88:
        case BUILDING_UNUSED_NATIVE_MEETING_89:
        case BUILDING_UNUSED_NATIVE_CROPS_93:
        case BUILDING_RESERVER_MISSION_POST_80:
        case BUILDING_POLICE_STATION:
        case BUILDING_ARCHITECT_POST:
        case BUILDING_FIREHOUSE:
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_ACADEMY:
        case BUILDING_LIBRARY:
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_MUD_TOWER:
        case BUILDING_FORT_CHARIOTEERS:
        case BUILDING_FORT_INFANTRY:
        case BUILDING_FORT_ARCHERS:
        case BUILDING_MILITARY_ACADEMY:
        case BUILDING_BAZAAR:
        case BUILDING_GRANARY:
        case BUILDING_SHIPWRIGHT:
        case BUILDING_DOCK:
        case BUILDING_FISHING_WHARF:
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_ROADBLOCK:
        case BUILDING_TAX_COLLECTOR_UPGRADED:
        case BUILDING_BREWERY_WORKSHOP:
        case BUILDING_WEAVER_WORKSHOP:
        case BUILDING_WEAPONSMITH:
        case BUILDING_JEWELS_WORKSHOP:
        case BUILDING_POTTERY_WORKSHOP:
            return 1;

        case BUILDING_BOOTH:
        case BUILDING_SENET_HOUSE:
        case BUILDING_PAVILLION:
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_MENU_BEAUTIFICATION:
            return 2;

        case BUILDING_RECRUITER:
        case BUILDING_BANDSTAND:
            return 3;

        case BUILDING_WELL:
        case BUILDING_SHRINE_OSIRIS:
        case BUILDING_SHRINE_RA:
        case BUILDING_SHRINE_PTAH:
        case BUILDING_SHRINE_SETH:
        case BUILDING_SHRINE_BAST:
            return 4;

        default:
            return 0;
        }
    }
    return 0;
}

static void get_tooltip(tooltip_context* c) {
    auto &context = g_building_info_context;
    std::pair<int, int> tooltip{-1, -1};
    
    if (context.type == BUILDING_INFO_LEGION) {
        tooltip.second = window_building_get_legion_info_tooltip_text(&context);

    } else if (context.type == BUILDING_INFO_BUILDING && context.storage_show_special_orders) {
        building *b = building_get(context.building_id);
        switch (b->type) {
        case BUILDING_STORAGE_YARD:
            window_building_get_tooltip_warehouse_orders(&tooltip.first, &tooltip.second);
            break;

        default:
            tooltip = b->dcast()->get_tooltip();
            break;
        }
    }

    int button_id = ui::button_hover(mouse_get());
    if (button_id > 0 && tooltip.first < 0) {
        tooltip = ui::button(button_id - 1)._tooltip;
    }

    if (tooltip.first || tooltip.second) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = tooltip.second;
        if (tooltip.first) {
            c->text_group = tooltip.first;
        }
        window_invalidate();
    }
}

static int center_in_city(int element_width_pixels) {
    vec2i view_pos, view_size;
    const view_data_t &viewport = city_view_viewport();
    city_view_get_viewport(viewport, view_pos, view_size);
    int margin = (view_size.x - element_width_pixels) / 2;
    return view_pos.x + margin;
}

void highlight_waypoints(building* b) { // highlight the 4 routing tiles for roams from this building
    map_clear_highlights();
    if (b->has_road_access) {
        map_highlight_set(b->road_access.grid_offset(), 2);
    }
    if (building_is_fort(b->type) || b->house_size) { // building doesn't send roamers
        return;
    }
    int hx, hy;
    map_point road_tile;
    hx = b->tile.x();
    hy = b->tile.y() - 8;
    map_grid_bound(&hx, &hy);
    if (map_closest_road_within_radius(tile2i(hx, hy), 1, 6, road_tile)) {
        map_highlight_set(road_tile.grid_offset(), 1);
    }

    hx = b->tile.x() + 8;
    hy = b->tile.y();
    map_grid_bound(&hx, &hy);
    if (map_closest_road_within_radius(tile2i(hx, hy), 1, 6, road_tile)) {
        map_highlight_set(road_tile.grid_offset(), 1);
    }

    hx = b->tile.x();
    hy = b->tile.y() + 8;
    map_grid_bound(&hx, &hy);
    if (map_closest_road_within_radius(tile2i(hx, hy), 1, 6, road_tile)) {
        map_highlight_set(road_tile.grid_offset(), 1);
    }

    hx = b->tile.x() - 8;
    hy = b->tile.y();
    map_grid_bound(&hx, &hy);
    if (map_closest_road_within_radius(tile2i(hx, hy), 1, 6, road_tile)) {
        map_highlight_set(road_tile.grid_offset(), 1);
    }

    window_invalidate();
}

int OFFSET(int x, int y) {
    return GRID_OFFSET(x, y);
}

static void init(map_point tile) {
    auto &context = g_building_info_context;
    const int grid_offset = tile.grid_offset();
    context.can_play_sound = true;
    context.storage_show_special_orders = 0;
    context.go_to_advisor = {ADVISOR_NONE, ADVISOR_NONE, ADVISOR_NONE};
    context.building_id = map_building_at(grid_offset);
    context.rubble_building_type = map_rubble_building_type(grid_offset);
    context.has_reservoir_pipes = map_terrain_is(grid_offset, TERRAIN_GROUNDWATER);
    context.aqueduct_has_water = map_aqueduct_at(grid_offset) && map_image_at(grid_offset) - image_id_from_group(GROUP_BUILDING_AQUEDUCT) < 15;

    city_resource_determine_available();
    context.type = BUILDING_INFO_TERRAIN;
    context.figure.drawn = 0;
    context.figure.draw_debug_path = 0;
    context.show_overlay = OVERLAY_NONE;

    if (!context.building_id && map_sprite_animation_at(grid_offset) > 0) {
        if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
            context.terrain_type = TERRAIN_INFO_BRIDGE;
        } else {
            context.terrain_type = TERRAIN_INFO_EMPTY;
        }

    } else if (map_property_is_plaza_or_earthquake(grid_offset)) {
        if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
            context.terrain_type = TERRAIN_INFO_PLAZA;
        }

        if (map_terrain_is(grid_offset, TERRAIN_ROCK)) {
            context.terrain_type = TERRAIN_INFO_EARTHQUAKE;
        }

    } else if (map_terrain_is(grid_offset, TERRAIN_TREE)) {
        context.terrain_type = TERRAIN_INFO_TREE;

    } else if (!context.building_id && map_terrain_is(grid_offset, TERRAIN_FLOODPLAIN)) {
        if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
            context.terrain_type = TERRAIN_INFO_FLOODPLAIN_SUBMERGED;
        } else if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
            context.terrain_type = TERRAIN_INFO_ROAD;
        } else {
            context.terrain_type = TERRAIN_INFO_FLOODPLAIN;
        }

    } else if (map_terrain_is(grid_offset, TERRAIN_MARSHLAND)) {
        context.terrain_type = TERRAIN_INFO_MARSHLAND;

    } else if (map_terrain_is(grid_offset, TERRAIN_DUNE)) {
        context.terrain_type = TERRAIN_INFO_DUNES;

    } else if (map_terrain_is(grid_offset, TERRAIN_ROCK)) {
        if (grid_offset == city_map_entry_flag().grid_offset()) {
            context.terrain_type = TERRAIN_INFO_ENTRY_FLAG;
        } else if (grid_offset == city_map_exit_flag().grid_offset()) {
            context.terrain_type = TERRAIN_INFO_EXIT_FLAG;
        } else {
            if (map_terrain_is(grid_offset, TERRAIN_ORE)) {
                context.terrain_type = TERRAIN_INFO_ORE_ROCK;
            } else {
                context.terrain_type = TERRAIN_INFO_ROCK;
            }
        }
    } else if ((map_terrain_get(grid_offset) & (TERRAIN_WATER | TERRAIN_BUILDING)) == TERRAIN_WATER) {
        context.terrain_type = TERRAIN_INFO_WATER;

    } else if (map_terrain_is(grid_offset, TERRAIN_SHRUB)) {
        context.terrain_type = TERRAIN_INFO_SHRUB;

    } else if (map_terrain_is(grid_offset, TERRAIN_GARDEN)) {
        context.terrain_type = TERRAIN_INFO_GARDEN;

    } else if ((map_terrain_get(grid_offset) & (TERRAIN_ROAD | TERRAIN_BUILDING)) == TERRAIN_ROAD) {
        context.terrain_type = TERRAIN_INFO_ROAD;

    } else if (map_terrain_is(grid_offset, TERRAIN_CANAL)) {
        context.terrain_type = TERRAIN_INFO_AQUEDUCT;

    } else if (map_terrain_is(grid_offset, TERRAIN_RUBBLE)) {
        context.terrain_type = TERRAIN_INFO_RUBBLE;

    } else if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
        context.terrain_type = TERRAIN_INFO_WALL;

    } else if (!context.building_id) {
        context.terrain_type = TERRAIN_INFO_EMPTY;

    } else {
        building* b = building_get(context.building_id);
        context.type = BUILDING_INFO_BUILDING;
        context.worker_percentage = calc_percentage<int>(b->num_workers, model_get_building(b->type)->laborers);
        highlight_waypoints(b);

        switch (b->type) {
        case BUILDING_FORT_GROUND:
            context.building_id = b->prev_part_building_id;
            // fallthrough

        case BUILDING_FORT_ARCHERS:
        case BUILDING_FORT_CHARIOTEERS:
        case BUILDING_FORT_INFANTRY:
            context.formation_id = b->formation_id;
            break;

        case BUILDING_STORAGE_ROOM:
        case BUILDING_SENET_HOUSE:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
            b = b->main();
            context.building_id = b->id;
            break;

        default:
            if (b->house_size) {
                context.worst_desirability_building_id = building_house_determine_worst_desirability_building(b);
                building_house_determine_evolve_text(b, context.worst_desirability_building_id);
            }
            break;
        }

        context.has_road_access = 0;
        switch (b->type) {
            //            case BUILDING_GRANARY:
            //                if (map_has_road_access_granary(b->tile.x(), b->tile.y(), 0))
            //                    context.has_road_access = 1;
            //
            //                break;
            //            case BUILDING_SENET_HOUSE:
            //                if (map_has_road_access_hippodrome_rotation(b->tile.x(), b->tile.y(), 0,
            //                b->subtype.orientation))
            //                    context.has_road_access = 1;

            //            case BUILDING_TEMPLE_COMPLEX_OSIRIS:
            //            case BUILDING_TEMPLE_COMPLEX_RA:
            //            case BUILDING_TEMPLE_COMPLEX_PTAH:
            //            case BUILDING_TEMPLE_COMPLEX_SETH:
            //            case BUILDING_TEMPLE_COMPLEX_BAST:
            //                if (map_has_road_access_hippodrome_rotation(b->tile.x(), b->tile.y(), 0,
            //                b->subtype.orientation))
            //                    context.has_road_access = 1;
            //
            //                break;
            //            case BUILDING_WAREHOUSE:
            //                if (map_has_road_access_rotation(b->subtype.orientation, b->tile.x(), b->tile.y(), 3, 0))
            //                    context.has_road_access = 1;
            //
            //                context.warehouse_space_text = building_warehouse_get_space_info(b);
            //                break;
        default:
            context.show_overlay = b->get_overlay();
            context.has_road_access = b->has_road_access;
            break;
        }
    }
    // figures
    context.figure.selected_index = 0;
    context.figure.count = 0;
    for (int i = 0; i < 7; i++) {
        context.figure.figure_ids[i] = 0;
    }
    const int FIGURE_OFFSETS[] = {OFFSET(0, 0),
                                  OFFSET(0, -1),
                                  OFFSET(0, 1),
                                  OFFSET(1, 0),
                                  OFFSET(-1, 0),
                                  OFFSET(-1, -1),
                                  OFFSET(1, -1),
                                  OFFSET(-1, 1),
                                  OFFSET(1, 1)};
    for (int i = 0; i < 9 && context.figure.count < 7; i++) {
        int figure_id = map_figure_id_get(grid_offset + FIGURE_OFFSETS[i]);
        while (figure_id > 0 && context.figure.count < 7) {
            figure* f = figure_get(figure_id);
            if (f->state != FIGURE_STATE_DEAD && f->action_state != FIGURE_ACTION_149_CORPSE) {
                switch (f->type) {
                case FIGURE_NONE:
                case FIGURE_EXPLOSION:
                case FIGURE_MAP_FLAG:
                case FIGURE_FLOTSAM:
                case FIGURE_ARROW:
                case FIGURE_JAVELIN:
                case FIGURE_BOLT:
                case FIGURE_BALLISTA:
                case FIGURE_CREATURE:
                case FIGURE_FISHING_POINT:
                case FIGURE_SPEAR:
                case FIGURE_CHARIOR_RACER:
                    break;

                default:
                    context.figure.figure_ids[context.figure.count++] = figure_id;
                    //                        f->igure_phrase_determine();
                    break;
                }
            }
            if (figure_id != f->next_figure)
                figure_id = f->next_figure;
            else
                figure_id = 0;
        }
    }
    // check for legion figures
    for (int i = 0; i < 7; i++) {
        int figure_id = context.figure.figure_ids[i];
        if (figure_id <= 0)
            continue;

        figure* f = figure_get(figure_id);
        if (f->type == FIGURE_STANDARD_BEARER || f->is_legion()) {
            context.type = BUILDING_INFO_LEGION;
            context.formation_id = f->formation_id;
            const formation* m = formation_get(context.formation_id);
            if (m->figure_type != FIGURE_STANDARD_BEARER)
                context.formation_types = 5;
            else if (m->has_military_training)
                context.formation_types = 4;
            else {
                context.formation_types = 3;
            }
            break;
        }
    }
    // dialog size
    context.width_blocks = 29;
    switch (get_height_id()) {
    case 1:
        context.height_blocks = 16;
        break;
    case 2:
        context.height_blocks = 18;
        break;
    case 3:
        context.height_blocks = 19;
        break;
    case 4:
        context.height_blocks = 14;
        break;
    case 5:
        context.height_blocks = 23;
        break;
    default:
        context.height_blocks = 22;
        break;
    }
    // dialog placement
    int s_width = screen_width();
    int s_height = screen_height();
    context.offset.x = center_in_city(16 * context.width_blocks);
    if (s_width >= 1024 && s_height >= 768) {
        context.offset.x = mouse_get()->x;
        context.offset.y = mouse_get()->y;
        window_building_set_possible_position(&context.offset.x, &context.offset.y, context.width_blocks, context.height_blocks);
    } else if (s_height >= 600 && mouse_get()->y <= (s_height - 24) / 2 + 24) {
        context.offset.y = s_height - 16 * context.height_blocks - MARGIN_POSITION;
    } else {
        context.offset.y = MIN_Y_POSITION;
    }
}

static void draw_refresh_background() {
    auto &context = g_building_info_context;
    if (context.type == BUILDING_INFO_NONE) {
        window_building_draw_no_people(&context);
    } else if (context.type == BUILDING_INFO_TERRAIN) {
        window_building_draw_terrain(&context);
    } else if (context.type == BUILDING_INFO_BUILDING) {
        if (building_is_house(building_get(context.building_id)->type)) {
            window_building_draw_house(&context);
        } else {
            switch (building_get(context.building_id)->type) {
            case BUILDING_LIMESTONE_QUARRY: building_limestone_quarry_draw_info(context); break;
            case BUILDING_WOOD_CUTTERS: building_timber_yard_draw_info(context); break;
            case BUILDING_SANDSTONE_QUARRY: building_sandstone_quarry_draw_info(context); break;
            case BUILDING_GRANITE_QUARRY: building_granite_quarry_draw_info(context); break;;
            case BUILDING_WEAVER_WORKSHOP: building_flax_workshop_draw_info(context); break;
            case BUILDING_JEWELS_WORKSHOP: building_luxury_workshop_draw_info(context); break;
            case BUILDING_BRICKS_WORKSHOP: building_brick_maker_workshop_draw_info(context); break;
            case BUILDING_SENET_HOUSE: window_building_draw_senet_house(&context); break;
            case BUILDING_PAVILLION: window_building_draw_pavilion(&context); break;
            case BUILDING_DANCE_SCHOOL: building_dancer_school_draw_info(context); break;
            case BUILDING_SENET_MASTER: building_bullfight_school_draw_info(context); break;
            case BUILDING_MORTUARY: building_mortuary_draw_info(context); break;
            case BUILDING_DENTIST: building_dentist_draw_info(context); break;
            case BUILDING_SCRIBAL_SCHOOL: building_scribal_school_draw_info(context); break;
            case BUILDING_ACADEMY: building_academy_draw_info(context); break;
            case BUILDING_LIBRARY: building_library_draw_info(context); break;
            
            case BUILDING_ORACLE:
                window_building_draw_oracle(&context);
                break;

            case BUILDING_SHIPWRIGHT: building_shipyard_draw_info(context); break;
            
            case BUILDING_DOCK:
                if (context.storage_show_special_orders)
                    window_building_draw_dock_orders(&context);
                else
                    window_building_draw_dock(&context);
                break;

            case BUILDING_FISHING_WHARF: building_wharf_draw_info(context); break;
            case BUILDING_WATER_LIFT: window_building_draw_water_lift(&context); break;
            case BUILDING_MENU_BEAUTIFICATION: window_building_draw_fountain(&context); break;

            case BUILDING_RESERVED_TRIUMPHAL_ARCH_56: window_building_draw_triumphal_arch(&context); break;
            case BUILDING_POLICE_STATION: window_building_draw_prefect(&context); break;
            
            case BUILDING_ROADBLOCK:
                if (context.storage_show_special_orders)
                    window_building_draw_roadblock_orders(&context);
                else
                    window_building_draw_roadblock(&context);
                break;

            case BUILDING_FERRY: window_building_draw_ferry(&context); break;
            case BUILDING_MUD_GATEHOUSE: window_building_draw_gatehouse(&context); break;
            case BUILDING_MUD_TOWER: window_building_draw_tower(&context); break;
            case BUILDING_MILITARY_ACADEMY: window_building_draw_military_academy(&context); break;
            
            case BUILDING_FORT_ARCHERS:
            case BUILDING_FORT_CHARIOTEERS:
            case BUILDING_FORT_INFANTRY:
                window_building_draw_fort(&context);
                break;
            
            case BUILDING_BURNING_RUIN: window_building_draw_burning_ruin(&context); break;
            case BUILDING_UNUSED_NATIVE_HUT_88: window_building_draw_native_hut(&context); break;
            case BUILDING_UNUSED_NATIVE_MEETING_89: window_building_draw_native_meeting(&context); break;
            case BUILDING_UNUSED_NATIVE_CROPS_93: window_building_draw_native_crops(&context); break;
            case BUILDING_RESERVER_MISSION_POST_80: window_building_draw_mission_post(&context); break;
            case BUILDING_FESTIVAL_SQUARE: window_building_draw_festival_square(&context); break;

            default:
                {
                    building *b = building_get(context.building_id);
                    b->dcast()->window_info_background(context);
                }
                break;
            }
        }
    } else if (context.type == BUILDING_INFO_LEGION) {
        window_building_draw_legion_info(&context);
    }
}

static void draw_background() {
    game.animation = false;
    window_city_draw_panels();
    window_city_draw();
    draw_refresh_background();
}

static void draw_foreground() {
    ui::begin_widget(g_building_info_context.offset);
    auto &context = g_building_info_context;

    // building-specific buttons
    building *b = nullptr;
    if (context.type == BUILDING_INFO_BUILDING) {
        b = building_get(context.building_id);
        switch (b->type) {
        case BUILDING_ROADBLOCK:
            if (context.storage_show_special_orders)
                window_building_draw_roadblock_orders_foreground(&context);
            else
                window_building_draw_roadblock_foreground(&context);
            break;

        case BUILDING_DOCK:
            if (context.storage_show_special_orders)
                window_building_draw_dock_orders_foreground(&context);
            else
                window_building_draw_dock_foreground(&context);
            break;

        default:
            b->dcast()->window_info_foreground(context);
            break;
        }
    } else if (context.type == BUILDING_INFO_LEGION) {
        window_building_draw_legion_info_foreground(&context);
    }

    // general buttons
    int y_offset = (context.storage_show_special_orders) ? context.subwnd_y_offset : 0;
    int height_blocks = (context.storage_show_special_orders) ? context.height_blocks_submenu : context.height_blocks;

    ui::img_button(GROUP_CONTEXT_ICONS, vec2i(14, y_offset + 16 * height_blocks - 40), {28, 28}, 0)
               .onclick([&context] (int, int) {
                    if (context.help_id > 0) {
                        window_message_dialog_show(context.help_id, -1, window_city_draw_all);
                    } else {
                        window_message_dialog_show(MESSAGE_DIALOG_HELP, -1, window_city_draw_all);
                    }
                    window_invalidate();
               });

    ui::img_button(GROUP_CONTEXT_ICONS, vec2i(16 * context.width_blocks - 40, y_offset + 16 * height_blocks - 40), {28, 28}, 4)
               .onclick([&context] (int, int) {
                    if (context.storage_show_special_orders) {
                        context.storage_show_special_orders = 0;
                        storage_settings_backup_reset();
                        window_invalidate();
                    } else {
                        window_city_show();
                    }
               });

    if (!context.storage_show_special_orders && context.go_to_advisor.first && is_advisor_available(context.go_to_advisor.first)) {
        int img_offset = (context.go_to_advisor.left_a - 1) * 3;
        ui::img_button(GROUP_MESSAGE_ADVISOR_BUTTONS, vec2i(40, 16 * context.height_blocks - 40), {28, 28}, img_offset)
               .onclick([&context] (int, int) {
                   window_advisors_show_advisor(context.go_to_advisor.first);
               });
    }

    if (!context.storage_show_special_orders && context.go_to_advisor.left_a && is_advisor_available(context.go_to_advisor.left_a)) {
        int img_offset = (context.go_to_advisor.left_a - 1) * 3;
        ui::img_button(GROUP_MESSAGE_ADVISOR_BUTTONS, vec2i(40, 16 * context.height_blocks - 40), {28, 28}, img_offset)
               .onclick([&context] (int, int) {
                   window_advisors_show_advisor(context.go_to_advisor.left_a);
               });
    }

    if (!context.storage_show_special_orders && context.go_to_advisor.left_b && is_advisor_available(context.go_to_advisor.left_b)) {
        int img_offset = (context.go_to_advisor.left_b - 1) * 3;
        ui::img_button(GROUP_MESSAGE_ADVISOR_BUTTONS, vec2i(65, 16 * context.height_blocks - 40), {28, 28}, img_offset)
               .onclick([&context] (int, int) {
                   window_advisors_show_advisor(context.go_to_advisor.left_b);
               });
    }

    if (!context.storage_show_special_orders && b) {
        int workers_needed = model_get_building(b->type)->laborers;
        if (workers_needed) {
            pcstr label = (b->state == BUILDING_STATE_VALID ? "x" : "");
            auto tooltip = (b->state == BUILDING_STATE_VALID) ? std::pair{54, 16} : std::pair{54, 17};
            ui::button(label, {400, 3 + 16 * context.height_blocks - 40}, {20, 20})
               .onclick([&context, b, workers_needed] (int, int) {
                   if (workers_needed) {
                       building_mothball_toggle(b);
                       window_invalidate();
                   }
               })
               .tooltip(tooltip);
        }
    }

    if (!context.storage_show_special_orders && context.figure.draw_debug_path) {
        figure* f = figure_get(context.figure.figure_ids[0]);
        pcstr label = (f->draw_debug_mode ? "P" : "p");
        ui::button(label, {400, 3 + 16 * context.height_blocks - 40}, {20, 20})
              .onclick([&context, f] (int, int) {
                  f->draw_debug_mode = f->draw_debug_mode ? 0 :FIGURE_DRAW_DEBUG_ROUTING;
                  window_invalidate();
              });
    }

    if (!context.storage_show_special_orders && context.show_overlay != OVERLAY_NONE) {
        pcstr label = (game.current_overlay != context.show_overlay ? "v" : "V");
        ui::button(label, {375, 3 + 16 * context.height_blocks - 40}, {20, 20})
             .onclick([&context] (int, int) {
                if (game.current_overlay != context.show_overlay) {
                    game_state_set_overlay((e_overlay)context.show_overlay);
                } else {
                    game_state_reset_overlay();
                }
                window_invalidate();
             });
    }
}

static int handle_specific_building_info_mouse(const mouse *m) {
    auto &context = g_building_info_context;
    // building-specific buttons
    if (context.type == BUILDING_INFO_NONE) {
        return 0;
    }

    if (context.type == BUILDING_INFO_LEGION) {
        return window_building_handle_mouse_legion_info(m, &context);
    } else if (context.figure.drawn) {
        return window_building_handle_mouse_figure_list(m, &context);
    } else if (context.type == BUILDING_INFO_BUILDING) {
        building *b = building_get(context.building_id);
        switch (building_get(context.building_id)->type) {
        case BUILDING_ROADBLOCK:
            if (context.storage_show_special_orders)
                return window_building_handle_mouse_roadblock_orders(m, &context);
            else
                return window_building_handle_mouse_roadblock(m, &context);
            break;

        case BUILDING_DOCK:
            if (context.storage_show_special_orders)
                return window_building_handle_mouse_dock_orders(m, &context);
            else
                return window_building_handle_mouse_dock(m, &context);
            break;

        case BUILDING_STORAGE_YARD:
            if (context.storage_show_special_orders)
                return window_building_handle_mouse_warehouse_orders(m, &context);
            else
                window_building_handle_mouse_warehouse(m, &context);
            break;

        default:
            return b->dcast()->window_info_handle_mouse(m, context);
        }
    }

    return 0;
}

static void handle_input(const mouse* m, const hotkeys* h) {
    auto &context = g_building_info_context;

    bool button_id = ui::handle_mouse(m);
  
    if (!button_id) {
        button_id |= !!handle_specific_building_info_mouse(m);
    }

    if (!button_id && input_go_back_requested(m, h)) {
        if (context.storage_show_special_orders) {
            storage_settings_backup_check();
        } else {
            window_city_show();
        }
    }
}

void window_building_info_show(const tile2i& point) {
    window_type window = {
        WINDOW_BUILDING_INFO,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip,
        draw_refresh_background
    };

    init(point);
    window_show(&window);
}

int window_building_info_get_int(void) {
    auto &context = g_building_info_context;
    if (context.type == BUILDING_INFO_BUILDING) {
        return building_get(context.building_id)->type;
    }

    return BUILDING_NONE;
}

void window_building_info_show_storage_orders(void) {
    auto &context = g_building_info_context;
    context.storage_show_special_orders = 1;
    window_invalidate();
}
