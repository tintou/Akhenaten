#include "boilerplate.h"

#include "building/construction/build_planner.h"
#include "building/building_granary.h"
#include "building/maintenance.h"
#include "building/monuments.h"
#include "building/menu.h"
#include "building/properties.h"
#include "building/storage.h"
#include "config/config.h"
#include "city/city_data.h"
#include "city/emperor.h"
#include "city/map.h"
#include "city/message.h"
#include "city/military.h"
#include "city/mission.h"
#include "city/resource.h"
#include "city/victory.h"
#include "core/bstring.h"
#include "content/vfs.h"
#include "empire/empire.h"
#include "empire/trade_prices.h"
#include "figure/enemy_army.h"
#include "figure/figure_names.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "figuretype/animal.h"
#include "figuretype/water.h"
#include "game/mission.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "game/undo.h"
#include "graphics/animation_timers.h"
#include "graphics/image.h"
#include "grid/aqueduct.h"
#include "grid/bookmark.h"
#include "grid/building.h"
#include "grid/desirability.h"
#include "grid/elevation.h"
#include "grid/figure.h"
#include "grid/image.h"
#include "grid/image_context.h"
#include "grid/orientation.h"
#include "grid/property.h"
#include "grid/random.h"
#include "grid/road_network.h"
#include "grid/routing/routing_terrain.h"
#include "grid/soldier_strength.h"
#include "grid/sprite.h"
#include "grid/terrain.h"
#include "grid/tiles.h"
#include "grid/floodplain.h"
#include "content/vfs.h"
#include "scenario/criteria.h"
#include "scenario/demand_change.h"
#include "scenario/distant_battle.h"
#include "scenario/earthquake.h"
#include "scenario/emperor_change.h"
#include "scenario/gladiator_revolt.h"
#include "scenario/invasion.h"
#include "scenario/price_change.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "sound/city.h"
#include "sound/music.h"
#include "widget/top_menu.h"
#include "window/city.h"
#include "window/file_dialog.h"
#include "window/mission_briefing.h"

#include "building/count.h"
#include "chunks.h"
#include "city/coverage.h"
#include "city/floods.h"
#include "io/io.h"
#include "io/manager.h"

#include <cassert>
#include <filesystem>

#ifdef _MSC_VER
// not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static const char MISSION_PACK_FILE[] = "mission1.pak";

bstring256 fullpath_saves(const char* filename) {
    if (strncasecmp(filename, "Save/", 5) == 0 || strncasecmp(filename, "Save\\", 5) == 0) {
        return bstring256(filename);
    }

    return bstring256(vfs::SAVE_FOLDER, "/", (const char*)g_settings.player_name, "/", filename);
}

void fullpath_maps(char* full, const char* filename) {
    strcpy(full, "");
    if (strncasecmp(filename, "Maps/", 5) == 0 || strncasecmp(filename, "Maps\\", 5) == 0) {
        strcat(full, filename);
        return;
    }
    strcat(full, "Maps/");
    strcat(full, filename);
}

static buffer* small_buffer = new buffer(4);
const int GamestateIO::get_campaign_scenario_offset(int scenario_id) {
    // init 4-byte buffer and read from file header corresponding to scenario index (i.e. mission 20 = offset 20*4 = 80)
    small_buffer->clear();
    if (!io_read_file_part_into_buffer(MISSION_PACK_FILE, NOT_LOCALIZED, small_buffer, 4, 4 * scenario_id))
        return 0;
    return small_buffer->read_i32();
}
const int GamestateIO::read_file_version(const char* filename, int offset) {
    small_buffer->clear();
    if (!io_read_file_part_into_buffer(filename, NOT_LOCALIZED, small_buffer, 4, offset + 4))
        return -1;
    return small_buffer->read_i32();
}

enum E_LOADED { LOADED_NULL = -1, LOADED_MISSION = 0, LOADED_SAVE = 1, LOADED_CUSTOM_MAP = 2 };

static int last_loaded = LOADED_NULL;
static void pre_load() { // do we NEED this...?
    scenario_set_campaign_scenario(-1);
    map_bookmarks_clear();

    // clear data
    city_victory_reset();
    Planner.reset();
    city_data_init();
    city_message_init_scenario();
    game_state_init();
    game_animation_init();
    sound_city_init();
    building_menu_enable_all();
    building_clear_all();
    building_storage_clear_all();
    figure_init_scenario();
    enemy_armies_clear();
    figure_name_init();
    formations_clear();
    figure_route_clear_all();
    map_clear_floodplain_growth();

    game_time_init(2098);
    map_monuments_clear();
    // clear grids
    map_image_clear();
    map_building_clear();
    map_terrain_clear();
    map_aqueduct_clear();
    map_figure_clear();
    map_property_clear();
    map_sprite_clear();
    map_random_clear();
    map_desirability_clear();
    map_elevation_clear();
    map_soldier_strength_clear();
    map_road_network_clear();

    map_image_context_init();
    map_random_init();
}

static void post_load() {
    // scenario settings
    scenario_set_name(scenario_name());
    city_set_player_name(g_settings.player_name);
    int scenario_id = scenario_campaign_scenario_id();
    int mission_rank = get_scenario_mission_rank(scenario_id);
    scenario_set_campaign_rank(mission_rank);

    // camera
    //    city_view_camera_position_refresh();
    //    city_view_refresh_viewport();

    // problems / overlays
    city_message_init_problem_areas();
    city_mission_tutorial_set_fire_message_shown(1);
    city_mission_tutorial_set_disease_message_shown(1);
    game_state_reset_overlay();
    game_undo_disable();

    // military
    image_set_enemy_pak(scenario_property_enemy());
    city_military_determine_distant_battle_city();
    scenario_distant_battle_set_kingdome_travel_months();
    scenario_distant_battle_set_enemy_travel_months();

    //map
    map_image_fix_icorrect_tiles();

    // building counts / storage
    // TODO: can't find cache in Pharaoh's save file format?
    building_count_update();
    building_granaries_calculate_stocks();
    city_resource_calculate_storageyard_stocks();
    city_resource_determine_available();
    for (int i = 0; i < building_get_highest_id(); i++) {
        building* b = building_get(i);
        if (b->state)
            b->update_road_access();
    }
    building_storage_reset_building_ids();
    city_culture_update_coverage();

    // traders / empire
    trade_prices_reset();
    city_emperor_init_scenario(scenario_campaign_rank());

    // city data special cases
    switch (last_loaded) {
    case LOADED_MISSION:
        city_data_init_campaign_mission();
        break;
    case LOADED_CUSTOM_MAP:
        city_data_init_custom_map();
        break;
    }

    // building menu
    building_menu_update(BUILDSET_NORMAL);

    // city messages
    city_message_clear_scroll();

    // city sounds
    sound_city_init();
}

// set up list of io_buffer chunks in correct order for specific file format read/write operations
static void file_schema(e_file_format file_format, const int file_version) {
    switch (file_format) {
    case FILE_FORMAT_MAP_FILE:
        FILEIO.push_chunk(4, false, "scenario_mission_index", iob_scenario_mission_id);
        FILEIO.push_chunk(4, false, "file_version", iob_file_version);
        FILEIO.push_chunk(6004, false, "chunks_schema", iob_chunks_schema);

        FILEIO.push_chunk(207936, false, "image_grid", &io_image_grid::instance());
        FILEIO.push_chunk(51984, false, "edge_grid", iob_edge_grid);
        FILEIO.push_chunk(207936, false, "terrain_grid", iob_terrain_grid);
        FILEIO.push_chunk(51984, false, "bitfields_grid", iob_bitfields_grid);
        FILEIO.push_chunk(51984, false, "random_grid", iob_random_grid);
        FILEIO.push_chunk(51984, false, "elevation_grid", iob_elevation_grid);

        FILEIO.push_chunk(8, false, "random_iv", iob_random_iv);
        FILEIO.push_chunk(8, false, "city_view_camera", iob_city_view_camera);
        FILEIO.push_chunk(1592, false, "scenario_info", iob_scenario_info);

        FILEIO.push_chunk(51984, false, "soil_fertility_grid", iob_soil_fertility_grid);
        FILEIO.push_chunk(18600, false, "scenario_events", iob_scenario_events);
        FILEIO.push_chunk(28, false, "scenario_events_extra", iob_scenario_events_extra);
        FILEIO.push_chunk(1280, true, "junk11", iob_junk11);
        FILEIO.push_chunk(file_version < 160 ? 15200 : 19600, true, "empire_map_objects", iob_empire_map_objects);
        FILEIO.push_chunk(16200, true, "empire_map_routes", iob_empire_map_routes);
        FILEIO.push_chunk(51984, false, "vegetation_growth", iob_vegetation_growth); // not sure what's the point of this in MAP...

        FILEIO.push_chunk(file_version < 147 ? 32 : 36, true, "floodplain_settings", iob_floodplain_settings);
        FILEIO.push_chunk(288, false, "trade_prices", iob_trade_prices);
        FILEIO.push_chunk(51984, true, "moisture_grid", iob_moisture_grid);

        break;
    case FILE_FORMAT_MISSION_PAK:
    case FILE_FORMAT_SAVE_FILE:
        FILEIO.push_chunk(4, false, "scenario_mission_index", iob_scenario_mission_id);
        FILEIO.push_chunk(4, false, "file_version", iob_file_version);
        FILEIO.push_chunk(6004, false, "chunks_schema", iob_chunks_schema);

        FILEIO.push_chunk(207936, true, "image_grid", &io_image_grid::instance());        // (228²) * 4 <<
        FILEIO.push_chunk(51984, true, "edge_grid", iob_edge_grid);                       // (228²) * 1
        FILEIO.push_chunk(103968, true, "building_grid", iob_building_grid);              // (228²) * 2
        FILEIO.push_chunk(207936, true, "terrain_grid", iob_terrain_grid);                // (228²) * 4 <<
        FILEIO.push_chunk(51984, true, "aqueduct_grid", iob_aqueduct_grid);               // (228²) * 1
        FILEIO.push_chunk(103968, true, "figure_grid", iob_figure_grid);                  // (228²) * 2
        FILEIO.push_chunk(51984, true, "bitfields_grid", iob_bitfields_grid);             // (228²) * 1
        FILEIO.push_chunk(51984, true, "sprite_grid", iob_sprite_grid);                   // (228²) * 1
        FILEIO.push_chunk(51984, false, "random_grid", iob_random_grid);                  // (228²) * 1
        FILEIO.push_chunk(51984, true, "desirability_grid", iob_desirability_grid);       // (228²) * 1
        FILEIO.push_chunk(51984, true, "elevation_grid", iob_elevation_grid);             // (228²) * 1
        FILEIO.push_chunk(103968, true, "building_damage_grid", iob_damage_grid);         // (228²) * 2 <<
        FILEIO.push_chunk(51984, true, "aqueduct_backup_grid", iob_aqueduct_backup_grid); // (228²) * 1
        FILEIO.push_chunk(51984, true, "sprite_backup_grid", iob_sprite_backup_grid);     // (228²) * 1
        FILEIO.push_chunk(776000, true, "figures", iob_figures);
        FILEIO.push_chunk(2000, true, "route_figures", iob_route_figures);
        FILEIO.push_chunk(500000, true, "route_paths", iob_route_paths);
        FILEIO.push_chunk(7200, true, "formations", iob_formations);
        FILEIO.push_chunk(12, false, "formations_info", iob_formations_info);
        FILEIO.push_chunk(37808, true, "city_data", iob_city_data);
        FILEIO.push_chunk(72, false, "city_data_extra", iob_city_data_extra);
        FILEIO.push_chunk(1056000, true, "buildings", iob_buildings);
        FILEIO.push_chunk(4, false, "city_view_orientation", iob_city_view_orientation);             // ok
        FILEIO.push_chunk(20, false, "game_time", iob_game_time);                                    // ok
        FILEIO.push_chunk(8, false, "building_extra_highest_id_ever", iob_building_highest_id_ever); // ok
        FILEIO.push_chunk(8, false, "random_iv", iob_random_iv);                                     // ok
        FILEIO.push_chunk(8, false, "city_view_camera", iob_city_view_camera);                       // ok
        //                state->building_count_culture1 = create_savegame_piece(132, false, ""); // MISSING
        FILEIO.push_chunk(8, false, "city_graph_order", iob_city_graph_order); // I guess ????
        //                state->emperor_change_time = create_savegame_piece(8, false, ""); // MISSING
        FILEIO.push_chunk(12, false, "empire_map_params", iob_empire_map_params);              // ok ???
        FILEIO.push_chunk(6466, true, "empire_cities", iob_empire_cities);                     // 83920 + 7681 --> 91601
        FILEIO.push_chunk(288, false, "building_count_industry", iob_building_count_industry); // 288 bytes ??????
        FILEIO.push_chunk(288, false, "trade_prices", iob_trade_prices);
        FILEIO.push_chunk(84, false, "figure_names", iob_figure_names);

        //                state->culture_coverage = create_savegame_piece(60, false, ""); // MISSING
        FILEIO.push_chunk(1592, false, "scenario_info", iob_scenario_info);

        /////////////////////

        FILEIO.push_chunk(4, false, "max_year", iob_max_year);
        FILEIO.push_chunk(48000, true, "messages", iob_messages);          // 94000 + 533 --> 94532 + 4 = 94536
        FILEIO.push_chunk(182, false, "message_extra", iob_message_extra); // ok

        FILEIO.push_chunk(8, false, "building_burning_list_info", iob_building_burning_list_info); // ok
        FILEIO.push_chunk(4, false, "figure_sequence", iob_figure_sequence);                       // ok
        FILEIO.push_chunk(12, false, "scenario_carry_settings", iob_scenario_carry_settings);      // ok
        FILEIO.push_chunk(3232, true, "invasion_warnings", iob_invasion_warnings); // 94743 + 31 --> 94774 + 4 = 94778
        FILEIO.push_chunk(4, false, "scenario_is_custom", iob_scenario_is_custom); // ok
        FILEIO.push_chunk(8960, false, "city_sounds", iob_city_sounds);            // ok
        FILEIO.push_chunk(4, false, "building_extra_highest_id", iob_building_highest_id); // ok
        FILEIO.push_chunk(8804, false, "figure_traders", iob_figure_traders);              // +4000 ???

        FILEIO.push_chunk(1000, true, "building_list_burning", iob_building_list_burning); // ok
        FILEIO.push_chunk(1000, true, "building_list_small", iob_building_list_small);     // ok
        FILEIO.push_chunk(8000, true, "building_list_large", iob_building_list_large);     // ok

        //                state->tutorial_part1 = create_savegame_piece(32, false, "");
        //                state->building_count_military = create_savegame_piece(16, false, "");
        //                state->enemy_army_totals = create_savegame_piece(20, false, "");
        //                state->building_storages = create_savegame_piece(6400, false, "");
        //                state->building_count_culture2 = create_savegame_piece(32, false, "");
        //                state->building_count_support = create_savegame_piece(24, false, "");
        //                state->tutorial_part2 = create_savegame_piece(4, false, "");
        //                state->gladiator_revolt = create_savegame_piece(16, false, "");

        // 32 bytes     00 00 00 00 ??? 8 x int
        // 24 bytes     00 00 00 00 ??? 6 x int
        FILEIO.push_chunk(32, false, "junk7a", iob_junk7a);                          // unknown bytes
        FILEIO.push_chunk(24, false, "junk7b", iob_junk7b);                          // unknown bytes
        FILEIO.push_chunk(39200, false, "building_storages", iob_building_storages); // storage instructions

        FILEIO.push_chunk(2880, true, "trade_routes_limits", iob_trade_routes_limits); // ok
        FILEIO.push_chunk(2880, true, "trade_routes_traded", iob_trade_routes_traded); // ok

        //                state->building_barracks_tower_sentry = create_savegame_piece(4, false, "");
        //                state->building_extra_sequence = create_savegame_piece(4, false, "");
        //                state->routing_counters = create_savegame_piece(16, false, "");
        //                state->building_count_culture3 = create_savegame_piece(40, false, "");
        //                state->enemy_armies = create_savegame_piece(900, false, "");

        // 12 bytes     00 00 00 00 ??? 3 x int
        // 16 bytes     00 00 00 00 ??? 4 x int
        // 12 bytes     00 00 00 00 ??? 3 x int
        //  2 bytes     00 00       ??? 1 x short
        //  8 bytes     00 00 00 00 ??? 2 x int
        FILEIO.push_chunk(50, false, "junk8", iob_routing_stats); // unknown bytes

        //                state->last_invasion_id = create_savegame_piece(2, false, "");
        //                state->building_extra_corrupt_houses = create_savegame_piece(8, false, "");

        FILEIO.push_chunk(65, false, "scenario_map_name", iob_scenario_map_name); // ok
        FILEIO.push_chunk(32, false, "bookmarks", iob_bookmarks);                 // ok

        // 12 bytes     00 00 00 00 ??? 3 x int
        // 396 bytes    00 00 00 00 ??? 99 x int
        FILEIO.push_chunk(12, false, "junk9a", iob_junk9a); // ok ????
        FILEIO.push_chunk(396, false, "junk9b", iob_junk9b);

        // 51984 bytes  00 00 00 00 ???
        FILEIO.push_chunk(51984, false, "soil_fertility_grid", iob_soil_fertility_grid);

        // 18600 bytes  00 00 00 00 ??? 150 x 124-byte chunk
        // 28 bytes     2F 01 00 00 ???
        FILEIO.push_chunk(18600, false, "scenario_events", iob_scenario_events);
        FILEIO.push_chunk(28, false, "scenario_events_extra", iob_scenario_events_extra);

        // 11000 bytes  00 00 00 00 ??? 50 x 224-byte chunk (50 x 220 for old version)
        // 2200 bytes   00 00 00 00 ??? 50 x 44-byte chunk
        // 16 bytes     00 00 00 00 ??? 4 x int
        // 8200 bytes   00 00 00 00 ??? 10 x 820-byte chunk
        FILEIO.push_chunk(file_version < 149 ? 11000 : 11200, false, "junk10a", iob_junk10a);
        FILEIO.push_chunk(2200, false, "junk10b", iob_junk10b);
        FILEIO.push_chunk(16, false, "junk10c", iob_junk10c);
        FILEIO.push_chunk(8200, false, "junk10d", iob_junk10d);

        // 1280 bytes   00 00 00 00 ??? 40 x 32-byte chunk
        FILEIO.push_chunk(1280, true, "junk11", iob_junk11); // unknown compressed data

        FILEIO.push_chunk(file_version < 160 ? 15200 : 19600, true, "empire_map_objects", iob_empire_map_objects);
        FILEIO.push_chunk(16200, true, "empire_map_routes", iob_empire_map_routes);

        // 51984 bytes  FF FF FF FF ???          // (228²) * 1 ?????????????????
        FILEIO.push_chunk(51984, false, "vegetation_growth", iob_vegetation_growth); // todo: 1-byte grid

        // 20 bytes     19 00 00 00 ???
        FILEIO.push_chunk(20, false, "junk14", iob_junk14);

        // 528 bytes    00 00 00 00 ??? 22 x 24-byte chunk
        FILEIO.push_chunk(528, false, "bizarre_ordered_fields_1", iob_bizarre_ordered_fields_1);

        FILEIO.push_chunk(file_version < 147 ? 32 : 36,
                          true,
                          "floodplain_settings",
                          iob_floodplain_settings);                        // floodplain_settings
        FILEIO.push_chunk(207936, true, "GRID03_32BIT", iob_GRID03_32BIT); // todo: 4-byte grid

        // 312 bytes    2B 00 00 00 ??? 13 x 24-byte chunk
        FILEIO.push_chunk(312,
                          false,
                          "bizarre_ordered_fields_4",
                          iob_bizarre_ordered_fields_4); // 71x 4-bytes emptiness

        // 64 bytes     00 00 00 00 ???
        FILEIO.push_chunk(64, false, "junk16", iob_junk16);                        // 71x 4-bytes emptiness
        FILEIO.push_chunk(41, false, "tutorial_flags_struct", iob_tutorial_flags); // 41 x 1-byte flag fields
        FILEIO.push_chunk(51984, true, "GRID04_8BIT", iob_GRID04_8BIT);

        // lone byte ???
        FILEIO.push_chunk(1, false, "junk17", iob_junk17);
        FILEIO.push_chunk(51984, true, "moisture_grid", iob_moisture_grid);

        // 240 bytes    0F 00 00 00 ??? 10 x 24-byte chunk
        // 432 bytes    0F 00 00 00 ??? 18 x 24-byte chunk
        FILEIO.push_chunk(240, false, "bizarre_ordered_fields_2", iob_bizarre_ordered_fields_2);
        FILEIO.push_chunk(432, false, "bizarre_ordered_fields_3", iob_bizarre_ordered_fields_3);

        // 8 bytes      00 00 00 00 ??? 2 x int
        FILEIO.push_chunk(8, false, "junk18", iob_junk18);

        if (file_version >= 160) {
            // 12 bytes     00 00 00 00 ??? 3 x int
            FILEIO.push_chunk(20, false, "junk19", iob_junk19);

            // 648 bytes   00 00 00 00 ??? 27 x 24-byte chunk
            // 648 bytes   00 00 00 00 ??? 27 x 24-byte chunk
            // 360 bytes   00 00 00 00 ??? 15 x 24-byte chunk
            // 1344 bytes  00 00 00 00 ??? 56 x 24-byte chunk
            // 1800 bytes  00 00 00 00 ??? 75 x 24-byte chunk <--- I can't even... their own schema is wrong. it's >>
            // 74! <<
            FILEIO.push_chunk(648, false, "bizarre_ordered_fields_5", iob_bizarre_ordered_fields_5);
            FILEIO.push_chunk(648, false, "bizarre_ordered_fields_6", iob_bizarre_ordered_fields_6);
            FILEIO.push_chunk(360, false, "bizarre_ordered_fields_7", iob_bizarre_ordered_fields_7);
            FILEIO.push_chunk(1344, false, "bizarre_ordered_fields_8", iob_bizarre_ordered_fields_8);
            FILEIO.push_chunk(1776, false, "bizarre_ordered_fields_9", iob_bizarre_ordered_fields_9);
        }
        break;
    case FILE_FORMAT_SAVE_FILE_EXT:
        FILEIO.push_chunk(4, false, "scenario_mission_index", iob_scenario_mission_id);
        FILEIO.push_chunk(4, false, "file_version", iob_file_version);
        FILEIO.push_chunk(6004, false, "chunks_schema", iob_chunks_schema);
        FILEIO.push_chunk(207936, false, "image_grid", &io_image_grid::instance());        // (228²) * 4 <<
        FILEIO.push_chunk(51984, false, "edge_grid", iob_edge_grid);                       // (228²) * 1
        FILEIO.push_chunk(103968, false, "building_grid", iob_building_grid);              // (228²) * 2
        FILEIO.push_chunk(207936, false, "terrain_grid", iob_terrain_grid);                // (228²) * 4 <<
        FILEIO.push_chunk(51984, false, "aqueduct_grid", iob_aqueduct_grid);               // (228²) * 1
        FILEIO.push_chunk(103968, false, "figure_grid", iob_figure_grid);                  // (228²) * 2
        FILEIO.push_chunk(51984, false, "bitfields_grid", iob_bitfields_grid);             // (228²) * 1
        FILEIO.push_chunk(51984, false, "sprite_grid", iob_sprite_grid);                   // (228²) * 1
        FILEIO.push_chunk(51984, false, "random_grid", iob_random_grid);                   // (228²) * 1
        FILEIO.push_chunk(51984, false, "desirability_grid", iob_desirability_grid);       // (228²) * 1
        FILEIO.push_chunk(51984, false, "elevation_grid", iob_elevation_grid);             // (228²) * 1
        FILEIO.push_chunk(103968, false, "building_damage_grid", iob_damage_grid);         // (228²) * 2 <<
        FILEIO.push_chunk(51984, false, "aqueduct_backup_grid", iob_aqueduct_backup_grid); // (228²) * 1
        FILEIO.push_chunk(51984, false, "sprite_backup_grid", iob_sprite_backup_grid);     // (228²) * 1
        FILEIO.push_chunk(776000, false, "figures", iob_figures);
        FILEIO.push_chunk(2000, false, "route_figures", iob_route_figures);
        FILEIO.push_chunk(500000, false, "route_paths", iob_route_paths);
        FILEIO.push_chunk(7200, false, "formations", iob_formations);
        FILEIO.push_chunk(12, false, "formations_info", iob_formations_info);
        FILEIO.push_chunk(37808, false, "city_data", iob_city_data);
        FILEIO.push_chunk(72, false, "city_data_extra", iob_city_data_extra);
        FILEIO.push_chunk(1056000, false, "buildings", iob_buildings);
        FILEIO.push_chunk(4, false, "city_view_orientation", iob_city_view_orientation);             // ok
        FILEIO.push_chunk(20, false, "game_time", iob_game_time);                                    // ok
        FILEIO.push_chunk(8, false, "building_extra_highest_id_ever", iob_building_highest_id_ever); // ok
        FILEIO.push_chunk(8, false, "random_iv", iob_random_iv);                                     // ok
        FILEIO.push_chunk(8, false, "city_view_camera", iob_city_view_camera);                       // ok
        FILEIO.push_chunk(8, false, "city_graph_order", iob_city_graph_order);                       // I guess ????
        FILEIO.push_chunk(12, false, "empire_map_params", iob_empire_map_params);                    // ok ???
        FILEIO.push_chunk(6466, false, "empire_cities", iob_empire_cities);                    // 83920 + 7681 --> 91601
        FILEIO.push_chunk(288, false, "building_count_industry", iob_building_count_industry); // 288 bytes ??????
        FILEIO.push_chunk(288, false, "trade_prices", iob_trade_prices);
        FILEIO.push_chunk(84, false, "figure_names", iob_figure_names);
        FILEIO.push_chunk(1592, false, "scenario_info", iob_scenario_info);
        FILEIO.push_chunk(4, false, "max_year", iob_max_year);
        FILEIO.push_chunk(48000, false, "messages", iob_messages);         // 94000 + 533 --> 94532 + 4 = 94536
        FILEIO.push_chunk(182, false, "message_extra", iob_message_extra); // ok
        FILEIO.push_chunk(8, false, "building_burning_list_info", iob_building_burning_list_info); // ok
        FILEIO.push_chunk(4, false, "figure_sequence", iob_figure_sequence);                       // ok
        FILEIO.push_chunk(12, false, "scenario_carry_settings", iob_scenario_carry_settings);      // ok
        FILEIO.push_chunk(3232, false, "invasion_warnings", iob_invasion_warnings); // 94743 + 31 --> 94774 + 4 = 94778
        FILEIO.push_chunk(4, false, "scenario_is_custom", iob_scenario_is_custom);  // ok
        FILEIO.push_chunk(8960, false, "city_sounds", iob_city_sounds);             // ok
        FILEIO.push_chunk(4, false, "building_extra_highest_id", iob_building_highest_id);  // ok
        FILEIO.push_chunk(8804, false, "figure_traders", iob_figure_traders);               // +4000 ???
        FILEIO.push_chunk(1000, false, "building_list_burning", iob_building_list_burning); // ok
        FILEIO.push_chunk(1000, false, "building_list_small", iob_building_list_small);     // ok
        FILEIO.push_chunk(8000, false, "building_list_large", iob_building_list_large);     // ok
        FILEIO.push_chunk(32, false, "junk7a", iob_junk7a);                                 // unknown bytes
        FILEIO.push_chunk(24, false, "junk7b", iob_junk7b);                                 // unknown bytes
        FILEIO.push_chunk(39200, false, "building_storages", iob_building_storages);        // storage instructions
        FILEIO.push_chunk(2880, false, "trade_routes_limits", iob_trade_routes_limits);     // ok
        FILEIO.push_chunk(2880, false, "trade_routes_traded", iob_trade_routes_traded);     // ok
        FILEIO.push_chunk(50, false, "junk8", iob_routing_stats);                           // unknown bytes
        FILEIO.push_chunk(65, false, "scenario_map_name", iob_scenario_map_name);           // ok
        FILEIO.push_chunk(32, false, "bookmarks", iob_bookmarks);                           // ok
        FILEIO.push_chunk(12, false, "junk9a", iob_junk9a);                                 // ok ????
        FILEIO.push_chunk(396, false, "junk9b", iob_junk9b);
        FILEIO.push_chunk(51984, false, "soil_fertility_grid", iob_soil_fertility_grid);
        FILEIO.push_chunk(18600, false, "scenario_events", iob_scenario_events);
        FILEIO.push_chunk(28, false, "scenario_events_extra", iob_scenario_events_extra);
        FILEIO.push_chunk(11200, false, "junk10a", iob_junk10a);
        FILEIO.push_chunk(2200, false, "junk10b", iob_junk10b);
        FILEIO.push_chunk(16, false, "junk10c", iob_junk10c);
        FILEIO.push_chunk(8200, false, "junk10d", iob_junk10d);
        FILEIO.push_chunk(1280, false, "junk11", iob_junk11); // unknown compressed data
        FILEIO.push_chunk(19600, true, "empire_map_objects", iob_empire_map_objects);
        FILEIO.push_chunk(16200, false, "empire_map_routes", iob_empire_map_routes);
        FILEIO.push_chunk(51984, false, "vegetation_growth", iob_vegetation_growth); // todo: 1-byte grid
        FILEIO.push_chunk(20, false, "junk14", iob_junk14);
        FILEIO.push_chunk(528, false, "bizarre_ordered_fields_1", iob_bizarre_ordered_fields_1);
        FILEIO.push_chunk(36, false, "floodplain_settings", iob_floodplain_settings); // floodplain_settings
        FILEIO.push_chunk(207936, false, "GRID03_32BIT", iob_GRID03_32BIT);           // todo: 4-byte grid
        FILEIO.push_chunk(312, false, "bizarre_ordered_fields_4", iob_bizarre_ordered_fields_4);                           // 71x 4-bytes emptiness
        FILEIO.push_chunk(64, false, "junk16", iob_junk16);                        // 71x 4-bytes emptiness
        FILEIO.push_chunk(41, false, "tutorial_flags_struct", iob_tutorial_flags); // 41 x 1-byte flag fields
        FILEIO.push_chunk(51984, false, "GRID04_8BIT", iob_GRID04_8BIT);
        FILEIO.push_chunk(1, false, "junk17", iob_junk17);
        FILEIO.push_chunk(51984, false, "moisture_grid", iob_moisture_grid);
        FILEIO.push_chunk(240, false, "bizarre_ordered_fields_2", iob_bizarre_ordered_fields_2);
        FILEIO.push_chunk(432, false, "bizarre_ordered_fields_3", iob_bizarre_ordered_fields_3);
        FILEIO.push_chunk(8, false, "junk18", iob_junk18);
        FILEIO.push_chunk(20, false, "junk19", iob_junk19);
        FILEIO.push_chunk(648, false, "bizarre_ordered_fields_5", iob_bizarre_ordered_fields_5);
        FILEIO.push_chunk(648, false, "bizarre_ordered_fields_6", iob_bizarre_ordered_fields_6);
        FILEIO.push_chunk(360, false, "bizarre_ordered_fields_7", iob_bizarre_ordered_fields_7);
        FILEIO.push_chunk(1344, false, "bizarre_ordered_fields_8", iob_bizarre_ordered_fields_8);
        FILEIO.push_chunk(1776, false, "bizarre_ordered_fields_9", iob_bizarre_ordered_fields_9);
        FILEIO.push_chunk(51984, false, "terrain_floodplain_growth", iob_terrain_floodplain_growth);
        FILEIO.push_chunk(207936, false, "monuments_progress", iob_monuments_progress_grid);
        break;
    }
}

bool GamestateIO::write_mission(const int scenario_id) {
    // TODO?
    return false;
}
bool GamestateIO::write_savegame(const char* filename_short) {
    bstring256 full = fullpath_saves(filename_short);

    // write file
    e_file_format format = get_format_from_file(filename_short);
    assert(format == FILE_FORMAT_SAVE_FILE_EXT);
    bool save_ok = FILEIO.serialize(full, 0, format, latest_save_version, file_schema);
    if (save_ok) {
        //vfs::path fs_path = vfs::content_path(full);
        config_set_string(CONFIG_STRING_LAST_SAVE, full);
        config_set_string(CONFIG_STRING_LAST_PLAYER, g_settings.player_name);
        config_save();
    }

    return save_ok;
}

bool GamestateIO::write_map(const char* filename_short) {
    return false; // TODO

    // concatenate string
    char full[MAX_FILE_NAME] = {0};
    fullpath_maps(full, filename_short);

    // write file
    return FILEIO.serialize(full, 0, FILE_FORMAT_MAP_FILE, 160, file_schema);
}

bool GamestateIO::load_mission(const int scenario_id, bool start_immediately) {
    // get mission pack file offset
    int offset = get_campaign_scenario_offset(scenario_id);
    if (offset <= 0) {
        return false;
    }

    // read file
    pre_load();
    if (!FILEIO.unserialize(MISSION_PACK_FILE, offset, FILE_FORMAT_MISSION_PAK, GamestateIO::read_file_version, file_schema)) {
        return false;
    }

    last_loaded = LOADED_MISSION;
    scenario_set_campaign_scenario(scenario_id);
    post_load();

    widget_top_menu_clear_state();

    // finish loading and start
    if (start_immediately) {
        start_loaded_file();

        // replay mission autosave file
        bstring256 filename("autosave_replay.", saved_game_data_expanded.extension);
        GamestateIO::write_savegame(filename);
    }

    return true;
}
bool GamestateIO::load_savegame(const char* filename_short, bool start_immediately) {
    // concatenate string
    bstring256 full = fullpath_saves(filename_short);

    // read file
    pre_load();
    e_file_format file_format = get_format_from_file(filename_short);
    if (!FILEIO.unserialize(full, 0, file_format, GamestateIO::read_file_version, file_schema)) {
        return false;
    }

    last_loaded = LOADED_SAVE;
    post_load();

    // finish loading and start
    if (start_immediately) {
        start_loaded_file();
    }

    return true;
}
bool GamestateIO::load_map(const char* filename_short, bool start_immediately) {
    // concatenate string
    char full[MAX_FILE_NAME] = {0};
    fullpath_maps(full, filename_short);

    // read file
    pre_load();
    if (!FILEIO.unserialize(full, 0, FILE_FORMAT_MAP_FILE, GamestateIO::read_file_version, file_schema)) {
        return false;
    }

    last_loaded = LOADED_CUSTOM_MAP;
    post_load();

    // finish loading and start
    if (start_immediately) {
        start_loaded_file();
        // replay mission autosave file
        GamestateIO::write_savegame("autosave_replay.sav");
    }

    return true;
}

void GamestateIO::start_loaded_file() {
    // build the map grids when loading MAP files
    if (last_loaded != LOADED_SAVE) {
        // initialize grids
        map_tiles_update_all_elevation();
        map_tiles_river_refresh_entire();
        map_tiles_update_all_earthquake();
        map_tiles_update_all_rocks();
        map_tiles_add_entry_exit_flags();
        map_tiles_update_all_cleared_land();
        map_tiles_update_all_empty_land();
        map_tiles_update_all_meadow();
        map_tiles_update_all_roads();
        map_tiles_update_all_plazas();
        map_tiles_update_all_walls();
        map_tiles_update_all_aqueducts(0);

        //        map_natives_init();
        figure_create_fishing_points();
        figure_create_herds();

        map_point entry = scenario_map_entry();
        map_point exit = scenario_map_exit();
        city_map_set_entry_point(entry.x(), entry.y());
        city_map_set_exit_point(exit.x(), exit.y());

        // game time
        game_time_init(scenario_property_start_year());

        // traders / empire
        empire_init_scenario();
        traders_clear();

        // set up events
        scenario_earthquake_init();
        scenario_gladiator_revolt_init();
        scenario_emperor_change_init();
        scenario_criteria_init_max_year();
        scenario_invasion_init();
        city_military_determine_distant_battle_city();
        scenario_request_init();
        scenario_demand_change_init();
        scenario_price_change_init();

        // tutorial flags
        tutorial_init();
    }

    scenario_load_meta_data(g_scenario_data.settings.campaign_scenario_id);
    building_properties_init();

    // city view / orientation
    city_view_init();
    city_settings_init();
    map_orientation_update_buildings();

    // river / garden tiles refresh
    build_terrain_caches();

    // routing
    map_routing_update_all();
    figure_route_clean();
    map_road_network_update();
    map_routing_update_ferry_routes();
    building_maintenance_check_kingdome_access();

    // tiles
    floodplains_init();
    map_tiles_update_floodplain_images();
    map_tiles_river_refresh_entire();
    map_tiles_determine_gardens();
    map_tiles_update_all_vegetation_tiles();
    map_building_update_all_tiles();

    if (last_loaded == LOADED_MISSION) {
        window_mission_briefing_show();
    } else {
        game_state_unpause();
        window_city_show();
    }
    sound_music_update(true);
    last_loaded = LOADED_NULL;
}

bool GamestateIO::delete_mission(const int scenario_id) {
    // TODO?
    return false;
}
bool GamestateIO::delete_savegame(const char* filename_short) {
    // concatenate string
    bstring256 full = fullpath_saves(filename_short);

    // delete file
    return vfs::file_remove(full);
}

bool GamestateIO::delete_map(const char* filename_short) {
    // concatenate string
    char full[MAX_FILE_NAME];
    fullpath_maps(full, filename_short);

    // delete file
    return vfs::file_remove(full);
}