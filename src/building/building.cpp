#include "building.h"

#include "building/properties.h"
#include "building/rotation.h"
#include "building/building_type.h"
#include "building/storage.h"
#include "building/building_temple.h"
#include "building/building_statue.h"
#include "building/building_barracks.h"
#include "building/building_bazaar.h"
#include "building/building_cattle_ranch.h"
#include "building/building_weaponsmith.h"
#include "building/building_tax_collector.h"
#include "building/building_physician.h"
#include "building/monument_mastaba.h"
#include "building/building_bandstand.h"
#include "building/building_bricklayers_guild.h"
#include "building/building_shrine.h"
#include "building/building_mansion.h"
#include "building/building_hunting_lodge.h"
#include "building/building_palace.h"
#include "building/destruction.h"
#include "city/buildings.h"
#include "city/population.h"
#include "widget/city/ornaments.h"
#include "city/warning.h"
#include "core/svector.h"
#include "core/profiler.h"
#include "figure/formation_legion.h"
#include "game/resource.h"
#include "game/undo.h"
#include "graphics/view/view.h"
#include "grid/building.h"
#include "grid/building_tiles.h"
#include "grid/desirability.h"
#include "grid/elevation.h"
#include "grid/grid.h"
#include "grid/random.h"
#include "grid/image.h"
#include "grid/routing/routing_terrain.h"
#include "grid/terrain.h"
#include "grid/tiles.h"
#include "io/io_buffer.h"
#include "graphics/graphics.h"
#include "menu.h"
#include "monuments.h"
#include "overlays/city_overlay.h"
#include "city/labor.h"

#include <string.h>

building g_all_buildings[5000];
std::span<building> g_city_buildings = make_span(g_all_buildings);

building_impl::static_params building_impl::static_params::dummy;

std::span<building>& city_buildings() {
    return g_city_buildings;
}

struct building_extra_data_t {
    int highest_id_in_use;
    int highest_id_ever;
    int created_sequence;
    //    int incorrect_houses;
    //    int unfixable_houses;
};

building_extra_data_t building_extra_data = {0, 0, 0};

int building_id_first(e_building_type type) {
    for (int i = 1; i < MAX_BUILDINGS; ++i) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == type)
            return i;
    }
    return MAX_BUILDINGS;
}

building* building_first(e_building_type type) {
    for (int i = 1; i < MAX_BUILDINGS; ++i) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == type)
            return b;
    }
    return nullptr;
}

building* building_next(int i, e_building_type type) {
    for (; i < MAX_BUILDINGS; ++i) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_VALID && b->type == type)
            return b;
    }
    return nullptr;
}

building* building_get(int id) {
    return &g_all_buildings[id];
}

void building::new_fill_in_data_for_type(e_building_type _tp, tile2i _tl, int orientation) {
    assert(!_ptr);
    const building_properties* props = building_properties_for_type(_tp);
    type = _tp;
    tile = _tl;
    state = BUILDING_STATE_CREATED;
    faction_id = 1;
    reserved_id = false; // city_buildings_unknown_value();
    size = props->size;
    creation_sequence_index = building_extra_data.created_sequence++;
    sentiment.house_happiness = 50;
    distance_from_entry = 0;

    map_random_7bit = map_random_get(tile.grid_offset()) & 0x7f;
    figure_roam_direction = map_random_7bit & 6;
    fire_proof = props->fire_proof;
    is_adjacent_to_water = map_terrain_is_adjacent_to_water(tile, size);

    // house size
    house_size = 0;
    if (type >= BUILDING_HOUSE_CRUDE_HUT && type <= BUILDING_HOUSE_SPACIOUS_APARTMENT) {
        house_size = 1;
    } else if (type >= BUILDING_HOUSE_COMMON_RESIDENCE && type <= BUILDING_HOUSE_FANCY_RESIDENCE) {
        house_size = 2;
    } else if (type >= BUILDING_HOUSE_COMMON_MANOR && type <= BUILDING_HOUSE_MEDIUM_PALACE) {
        house_size = 3;
    } else if (type >= BUILDING_HOUSE_LARGE_PALACE && type <= BUILDING_HOUSE_LUXURY_PALACE) {
        house_size = 4;
    }

    // subtype
    if (is_house()) {
        subtype.house_level = (e_house_level)(type - BUILDING_HOUSE_VACANT_LOT);
    } else {
        subtype.house_level = HOUSE_CRUDE_HUT;
    }

    // unique data
    switch (type) {
    case BUILDING_LIMESTONE_QUARRY:
        output_resource_first_id = RESOURCE_LIMESTONE;
        break;

    case BUILDING_WOOD_CUTTERS:
        output_resource_first_id = RESOURCE_TIMBER;
        data.industry.max_gatheres = 1;
        break;

    case BUILDING_WEAVER_WORKSHOP:
        data.industry.first_material_id = RESOURCE_FLAX;
        output_resource_first_id = RESOURCE_LINEN;
        break;

    case BUILDING_JEWELS_WORKSHOP:
        data.industry.first_material_id = RESOURCE_GEMS;
        output_resource_first_id = RESOURCE_LUXURY_GOODS;
        break;

    case BUILDING_SCRIBAL_SCHOOL:
        data.entertainment.consume_material_id = RESOURCE_PAPYRUS;
        break;

    case BUILDING_BRICKS_WORKSHOP:
        data.industry.first_material_id = RESOURCE_STRAW;
        data.industry.second_material_id = RESOURCE_CLAY;
        output_resource_first_id = RESOURCE_BRICKS;
        break;

    case BUILDING_CHARIOTS_WORKSHOP:
        data.industry.first_material_id = RESOURCE_TIMBER;
        data.industry.second_material_id = RESOURCE_WEAPONS;
        output_resource_first_id = RESOURCE_CHARIOTS;
        break;

    case BUILDING_GRANITE_QUARRY:
        output_resource_first_id = RESOURCE_GRANITE;
        break;

    case BUILDING_SANDSTONE_QUARRY:
        output_resource_first_id = RESOURCE_SANDSTONE;
        break;

    case BUILDING_HENNA_FARM:
        output_resource_first_id = RESOURCE_HENNA;
        break;

    case BUILDING_LAMP_WORKSHOP:
        data.industry.first_material_id = RESOURCE_OIL;
        data.industry.second_material_id = RESOURCE_TIMBER;
        output_resource_first_id = RESOURCE_LAMPS;
        break;

    case BUILDING_PAINT_WORKSHOP:
        data.industry.first_material_id = RESOURCE_OIL;
        output_resource_first_id = RESOURCE_PAINT;
        break;

    case BUILDING_BAZAAR: // Set it as accepting all goods
        subtype.market_goods = 0x0000;
        break;

    case BUILDING_TEMPLE_COMPLEX_OSIRIS:
    case BUILDING_TEMPLE_COMPLEX_RA:
    case BUILDING_TEMPLE_COMPLEX_PTAH:
    case BUILDING_TEMPLE_COMPLEX_SETH:
    case BUILDING_TEMPLE_COMPLEX_BAST:
        data.monuments.variant = (10 - (2 * orientation)) % 8; // ugh!
        break;

    case BUILDING_FISHING_WHARF:
        data.industry.orientation = orientation;
        output_resource_first_id = RESOURCE_FISH;
        break;

    case BUILDING_WATER_LIFT:
    case BUILDING_TRANSPORT_WHARF:
    case BUILDING_SHIPWRIGHT:
    case BUILDING_WARSHIP_WHARF:
    case BUILDING_FERRY:
        data.industry.orientation = orientation;
        break;

    case BUILDING_DOCK:
        data.dock.orientation = orientation;
        break;

    case BUILDING_BRICK_GATEHOUSE:
    case BUILDING_MUD_GATEHOUSE:
        subtype.orientation = orientation;
        break;

    default:
        output_resource_first_id = RESOURCE_NONE;
        dcast()->on_create();
        break;
    }
}

building* building_create(e_building_type type, tile2i tile, int orientation) {
    building* b = nullptr;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        if (g_all_buildings[i].state == BUILDING_STATE_UNUSED && !game_undo_contains_building(i)) {
            b = &g_all_buildings[i];
            break;
        }
    }

    if (!b) {
        city_warning_show(WARNING_DATA_LIMIT_REACHED);
        return &g_all_buildings[0];
    }

    b->clear_impl();

    memset(&(b->data), 0, sizeof(b->data));
    b->new_fill_in_data_for_type(type, tile, orientation);
    
    b->data.house.health = 100;

    return b;
}

building_impl *buildings::create(e_building_type e, building &data) {
    for (BuildingIterator *s = BuildingIterator::tail; s; s = s->next) {
        auto impl = s->func(e, data);
        if (impl) {
            return impl;
        }
    }

    return new building_impl(data);
}

building_impl *building::dcast() {
    if (_ptr) {
        return _ptr;
    }

    switch (type) {
    case BUILDING_BRICKLAYERS_GUILD: _ptr = new building_bricklayers_guild(*this); break;
    case BUILDING_BANDSTAND: _ptr = new building_bandstand(*this); break;
    case BUILDING_HUNTING_LODGE: _ptr = new building_hunting_lodge(*this); break;
    case BUILDING_PHYSICIAN: _ptr = new building_physician(*this); break;
    case BUILDING_WEAPONSMITH: _ptr = new building_weaponsmith(*this); break;
    case BUILDING_RECRUITER: _ptr = new building_recruiter(*this); break;
    case BUILDING_CATTLE_RANCH: _ptr = new building_cattle_ranch(*this); break;

    case BUILDING_TEMPLE_OSIRIS:
    case BUILDING_TEMPLE_RA:
    case BUILDING_TEMPLE_PTAH:
    case BUILDING_TEMPLE_SETH:
    case BUILDING_TEMPLE_BAST:
        _ptr = new building_temple(*this);
        break;

    case BUILDING_VILLAGE_PALACE:
    case BUILDING_TOWN_PALACE:
    case BUILDING_CITY_PALACE:
        _ptr = new building_palace(*this);
        break;

    case BUILDING_TAX_COLLECTOR:
    case BUILDING_TAX_COLLECTOR_UPGRADED:
        _ptr = new building_tax_collector(*this);
        break;

    case BUILDING_SMALL_STATUE:
    case BUILDING_MEDIUM_STATUE:
    case BUILDING_LARGE_STATUE:
        _ptr = new building_statue(*this);
        break;

    case BUILDING_SHRINE_OSIRIS:
    case BUILDING_SHRINE_RA:
    case BUILDING_SHRINE_PTAH:
    case BUILDING_SHRINE_SETH:
    case BUILDING_SHRINE_BAST:
        _ptr = new building_shrine(*this); 
        break;

    case BUILDING_PERSONAL_MANSION:
    case BUILDING_FAMILY_MANSION:
    case BUILDING_DYNASTY_MANSION:
        _ptr = new building_mansion(*this);
        break;
    
    case BUILDING_SMALL_MASTABA:
    case BUILDING_SMALL_MASTABA_SIDE:
    case BUILDING_SMALL_MASTABA_ENTRANCE:
    case BUILDING_SMALL_MASTABA_WALL:
         _ptr = new building_small_mastaba(*this);
         break;
    }

    if (!_ptr) {
        _ptr = buildings::create(type, *this);
    }
    return _ptr;
}

building_farm *building::dcast_farm() { return dcast()->dcast_farm(); }
building_brewery *building::dcast_brewery() { return dcast()->dcast_brewery(); }
building_pottery *building::dcast_pottery() { return dcast()->dcast_pottery(); }
building_storage_yard *building::dcast_storage_yard() { return dcast()->dcast_storage_yard(); }
building_storage_room *building::dcast_storage_room() { return dcast()->dcast_storage_room(); }
building_juggler_school *building::dcast_juggler_school() { return dcast()->dcast_juggler_school(); }
building_bazaar *building::dcast_bazaar() { return dcast()->dcast_bazaar(); }
building_firehouse *building::dcast_firehouse() { return dcast()->dcast_firehouse(); }
building_architect_post *building::dcast_architect_post() { return dcast()->dcast_architect_post(); }
building_booth *building::dcast_booth() { return dcast()->dcast_booth(); }
building_apothecary *building::dcast_apothecary() { return dcast()->dcast_apothecary(); }
building_granary *building::dcast_granary() { return dcast()->dcast_granary(); }
building_water_supply *building::dcast_water_supply() { return dcast()->dcast_water_supply(); }
building_conservatory *building::dcast_conservatory() { return dcast()->dcast_conservatory(); }
building_courthouse *building::dcast_courthouse() { return dcast()->dcast_courthouse(); }
building_well *building::dcast_well() { return dcast()->dcast_well(); }
building_clay_pit *building::dcast_clay_pit() { return dcast()->dcast_clay_pit(); }
building_reed_gatherer *building::dcast_reed_gatherer() { return dcast()->dcast_reed_gatherer(); }
building_papyrus_maker *building::dcast_papyrus_maker() { return dcast()->dcast_papyrus_maker(); }

building* building_at(int grid_offset) {
    return building_get(map_building_at(grid_offset));
}

building* building_at(int x, int y) {
    return building_get(map_building_at(MAP_OFFSET(x, y)));
}

building* building_at(tile2i point) {
    return building_get(map_building_at(point.grid_offset()));
}

bool building_exists_at(int grid_offset, building* b) {
    b = nullptr;
    int b_id = map_building_at(grid_offset);
    if (b_id > 0) {
        b = building_get(b_id);
        if (b->state > BUILDING_STATE_UNUSED) {
            return true;
        } else {
            b = nullptr;
        }
    }
    return false;
}
bool building_exists_at(tile2i tile, building* b) {
    b = nullptr;
    int b_id = map_building_at(tile);
    if (b_id > 0) {
        b = building_get(b_id);
        if (b->state > BUILDING_STATE_UNUSED)
            return true;
        else
            b = nullptr;
    }
    return false;
}

building::building() {
}

void building::industry_add_workers(int fid) {
    data.industry.worker_id = fid;
}

void building::industry_remove_worker(int fid) {
    if (data.industry.worker_id == id) {
        data.industry.worker_id = 0;
    }
}

void building::monument_add_workers(int fid) {
    for (auto &wid : data.monuments.workers) {
        if (wid == 0) {
            wid = fid;
            return;
        }
    }
}

building* building::main() {
    building* b = this;
    for (int guard = 0; guard < 99; guard++) {
        if (b->prev_part_building_id <= 0)
            return b;
        b = &g_all_buildings[b->prev_part_building_id];
    }
    return &g_all_buildings[0];
}

building* building::top_xy() {
    building* b = main();
    int x = b->tile.x();
    int y = b->tile.y();
    building* top = b;
    while (b->next_part_building_id <= 0) {
        b = next();
        if (b->tile.x() < x)
            top = b;
        if (b->tile.y() < y)
            top = b;
    }
    return top;
}

bool building::is_main() {
    return (prev_part_building_id == 0);
}

static void building_delete_UNSAFE(building* b) {
    b->clear_related_data();
    int id = b->id;
    memset(b, 0, sizeof(building));
    b->id = id;
}

void building::clear_impl() {
    delete _ptr;
    _ptr = nullptr;
}

void building::clear_related_data() {
    if (storage_id)
        building_storage_delete(storage_id);

    if (is_palace()) {
        city_buildings_remove_palace(this);
    }

    if (is_governor_mansion()) {
        city_buildings_remove_mansion(this);
    }

    if (type == BUILDING_DOCK)
        city_buildings_remove_dock();

    if (type == BUILDING_RECRUITER) {
        city_buildings_remove_recruiter(this);
    }

    if (type == BUILDING_DISTRIBUTION_CENTER_UNUSED)
        city_buildings_remove_distribution_center(this);

    if (building_is_fort(type)) {
        formation_legion_delete_for_fort(this);
    }

    if (type == BUILDING_SENET_HOUSE)
        city_buildings_remove_hippodrome();

    if (type == BUILDING_RESERVED_TRIUMPHAL_ARCH_56) {
        city_buildings_remove_triumphal_arch();
        building_menu_update(BUILDSET_NORMAL);
    }

    if (type == BUILDING_FESTIVAL_SQUARE) {
        city_buildings_remove_festival_square();
    }

    dcast()->on_destroy();
    clear_impl();
}

e_overlay building::get_overlay() const {
    switch (type) {
        case BUILDING_POLICE_STATION: return OVERLAY_CRIME;
        case BUILDING_SCRIBAL_SCHOOL: return OVERLAY_SCRIBAL_SCHOOL;
    }

    return const_cast<building*>(this)->dcast()->get_overlay();
}

void building_clear_all() {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        memset(&g_all_buildings[i], 0, sizeof(building));
        g_all_buildings[i].id = i;
    }
    building_extra_data.highest_id_in_use = 0;
    building_extra_data.highest_id_ever = 0;
    building_extra_data.created_sequence = 0;
    //    extra.incorrect_houses = 0;
    //    extra.unfixable_houses = 0;
}
// void building_totals_add_corrupted_house(int unfixable)
//{
//     extra.incorrect_houses++;
//     if (unfixable) {
//         extra.unfixable_houses++;
//     }
// }

bool building::is_house() {
    return building_is_house(type);
}
bool building::is_fort() {
    return building_is_fort(type);
}
bool building::is_defense() {
    return building_is_defense(type);
}
bool building::is_farm() {
    return building_is_farm(type);
}
bool building::is_workshop() {
    return building_is_workshop(type);
}
bool building::is_extractor() {
    return building_is_extractor(type);
}
bool building::is_monument() {
    return building_is_monument(type);
}
bool building::is_palace() {
    return building_is_palace(type);
}
bool building::is_tax_collector() {
    return building_is_tax_collector(type);
}
bool building::is_governor_mansion() {
    return building_is_governor_mansion(type);
}
bool building::is_temple() {
    return building_is_temple(type);
}
bool building::is_large_temple() {
    return building_is_large_temple(type);
}
bool building::is_shrine() const {
    return building_is_shrine(type);
}
bool building::is_guild() {
    return building_is_guild(type);
}
bool building::is_beautification() {
    return building_is_beautification(type);
}
bool building::is_water_crossing() {
    return building_is_water_crossing(type);
}

bool building::is_industry() {
    return building_is_industry(type);
}
bool building::is_food_category() {
    return building_is_food_category(type);
}
bool building::is_infrastructure() {
    return building_is_infrastructure(type);
}
bool building::is_administration() {
    return building_is_administration(type);
}
bool building::is_religion() {
    return building_is_religion(type);
}
bool building::is_entertainment() {
    return building_is_entertainment(type);
}
bool building::is_education() {
    return building_is_education(type);
}
bool building::is_military() {
    return building_is_military(type);
}

///////////////

bool building_is_fort(int type) {
    return type == BUILDING_FORT_CHARIOTEERS || type == BUILDING_FORT_ARCHERS || type == BUILDING_FORT_INFANTRY;
}

bool building_is_defense(e_building_type type) {
    return building_type_any_of(type, BUILDING_BRICK_WALL, BUILDING_BRICK_GATEHOUSE, BUILDING_BRICK_TOWER);
}

bool building_is_farm(e_building_type type) {
    return (type >= BUILDING_BARLEY_FARM && type <= BUILDING_CHICKPEAS_FARM) || type == BUILDING_FIGS_FARM
           || type == BUILDING_HENNA_FARM;
}

bool building_is_floodplain_farm(building &b) {
    return (building_is_farm(b.type) && map_terrain_is(b.tile.grid_offset(), TERRAIN_FLOODPLAIN)); // b->data.industry.labor_state >= 1 // b->labor_category == 255
}

bool building_is_workshop(int type) {
    return (type >= BUILDING_BREWERY_WORKSHOP && type <= BUILDING_POTTERY_WORKSHOP)
           || (type >= BUILDING_PAPYRUS_WORKSHOP && type <= BUILDING_CHARIOTS_WORKSHOP) || type == BUILDING_CATTLE_RANCH
           || type == BUILDING_LAMP_WORKSHOP || type == BUILDING_PAINT_WORKSHOP;
}

bool building_is_extractor(int type) {
    return (type == BUILDING_STONE_QUARRY || type == BUILDING_LIMESTONE_QUARRY || type == BUILDING_CLAY_PIT)
           || type == BUILDING_GOLD_MINE || type == BUILDING_GEMSTONE_MINE || type == BUILDING_COPPER_MINE
           || type == BUILDING_GRANITE_QUARRY || type == BUILDING_SANDSTONE_QUARRY;
}

bool building_is_harvester(e_building_type type) {
    return (type == BUILDING_REED_GATHERER || type == BUILDING_WOOD_CUTTERS);
}

bool building_is_monument(int type) {
    switch (type) {
    case BUILDING_SMALL_MASTABA:
    case BUILDING_SMALL_MASTABA_SIDE:
    case BUILDING_SMALL_MASTABA_WALL:
    case BUILDING_SMALL_MASTABA_ENTRANCE:
    case BUILDING_PYRAMID:
    case BUILDING_SPHINX:
    case BUILDING_MAUSOLEUM:
    case BUILDING_ALEXANDRIA_LIBRARY:
    case BUILDING_CAESAREUM:
    case BUILDING_PHAROS_LIGHTHOUSE:
    case BUILDING_SMALL_ROYAL_TOMB:
    case BUILDING_ABU_SIMBEL:
    case BUILDING_MEDIUM_ROYAL_TOMB:
    case BUILDING_LARGE_ROYAL_TOMB:
    case BUILDING_GRAND_ROYAL_TOMB:
    case BUILDING_SUN_TEMPLE:
        return true;

    default:
        return false;
    }
}
bool building_is_palace(e_building_type type) {
    return building_type_any_of(type, BUILDING_VILLAGE_PALACE, BUILDING_TOWN_PALACE, BUILDING_CITY_PALACE);
}

bool building_is_tax_collector(int type) {
    return (type >= BUILDING_TAX_COLLECTOR && type <= BUILDING_TAX_COLLECTOR_UPGRADED);
}

bool building_is_governor_mansion(int type) {
    return (type >= BUILDING_PERSONAL_MANSION && type <= BUILDING_DYNASTY_MANSION);
}

bool building_is_temple(int type) {
    return (type >= BUILDING_TEMPLE_OSIRIS && type <= BUILDING_TEMPLE_BAST);
}

bool building_is_large_temple(int type) {
    return (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST);
}

bool building_is_shrine(int type) {
    return (type >= BUILDING_SHRINE_OSIRIS && type <= BUILDING_SHRINE_BAST);
}
bool building_is_guild(e_building_type type) {
    return building_type_any_of(type, BUILDING_CARPENTERS_GUILD, BUILDING_STONEMASONS_GUILD, BUILDING_BRICKLAYERS_GUILD);
}
bool building_is_statue(int type) {
    return (type >= BUILDING_SMALL_STATUE && type <= BUILDING_LARGE_STATUE);
}
bool building_is_beautification(int type) {
    return building_is_statue(type) || type == BUILDING_GARDENS || type == BUILDING_PLAZA;
}
bool building_is_water_crossing(int type) {
    return (type == BUILDING_FERRY) || type == BUILDING_LOW_BRIDGE || type == BUILDING_UNUSED_SHIP_BRIDGE_83;
}
bool building_is_industry_type(const building* b) {
    return b->output_resource_first_id || building_type_any_of(b->type, BUILDING_SHIPWRIGHT, BUILDING_FISHING_WHARF);
}

bool building_is_industry(e_building_type type) {
    if (building_is_extractor(type))
        return true;
    if (building_is_harvester(type))
        return true;
    if (building_is_workshop(type))
        return true;
    if (building_is_farm(type))
        return true;
    if (building_is_guild(type))
        return true;
    if (type == BUILDING_DOCK || type == BUILDING_SHIPWRIGHT)
        return true;
    if (type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_ROOM)
        return true;
    return false;
}

bool building_is_food_category(e_building_type type) {
    if (building_is_farm(type)) { // special case for food-producing farms
        return (type >= BUILDING_GRAIN_FARM && type <= BUILDING_CHICKPEAS_FARM) || type == BUILDING_FIGS_FARM;
    }

    if (type == BUILDING_GRANARY || type == BUILDING_BAZAAR || type == BUILDING_WORK_CAMP
        || type == BUILDING_FISHING_WHARF || type == BUILDING_CATTLE_RANCH || type == BUILDING_HUNTING_LODGE) {
        return true;
    }

    return false;
}

bool building_is_infrastructure(int type) {
    if (type == BUILDING_ARCHITECT_POST || type == BUILDING_FIREHOUSE || type == BUILDING_POLICE_STATION)
        return true;

    if (building_is_water_crossing(type))
        return true;

    return false;
}

bool building_is_administration(e_building_type type) {
    if (building_is_palace(type) || building_is_tax_collector(type) || building_is_governor_mansion(type)) {
        return true;
    }

    if (type == BUILDING_COURTHOUSE) {
        return true;
    }

    return false;
}

bool building_is_religion(int type) {
    if (building_is_temple(type) || building_is_large_temple(type) || building_is_shrine(type))
        return true;

    if (type == BUILDING_FESTIVAL_SQUARE)
        return true;
    return false;
}

bool building_is_entertainment(int type) {
    if (type == BUILDING_BOOTH || type == BUILDING_BANDSTAND || type == BUILDING_PAVILLION) {
        return true;
    }

    if (type == BUILDING_JUGGLER_SCHOOL || type == BUILDING_CONSERVATORY || type == BUILDING_DANCE_SCHOOL) {
        return true;
    }

    if (type == BUILDING_SENET_HOUSE || type == BUILDING_ZOO) {
        return true;
    }

    return false;
}

bool building_is_education(e_building_type type) {
    return building_type_any_of(type, BUILDING_SCRIBAL_SCHOOL, BUILDING_LIBRARY, BUILDING_ACADEMY);
}

bool building_is_military(int type) {
    if (building_is_fort(type)) {
        return true;
    }
    
    if (building_type_any_of(BUILDING_MILITARY_ACADEMY, BUILDING_RECRUITER, BUILDING_FORT_GROUND)) {
        return true;
    }
    
    return false;
}

bool building_is_draggable(int type) {
    switch (type) {
    case BUILDING_CLEAR_LAND:
    case BUILDING_ROAD:
    case BUILDING_IRRIGATION_DITCH:
    case BUILDING_MUD_WALL:
    case BUILDING_PLAZA:
    case BUILDING_GARDENS:
    case BUILDING_HOUSE_VACANT_LOT:
        return true;

    case BUILDING_WATER_LIFT:
        return false;

    default:
        return false;
    }
}

int building_get_highest_id(void) {
    return building_extra_data.highest_id_in_use;
}
void building_update_highest_id(void) {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Update Highest Id");
    auto& extra = building_extra_data;

    building_extra_data.highest_id_in_use = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        if (g_all_buildings[i].state != BUILDING_STATE_UNUSED)
            extra.highest_id_in_use = i;
    }
    if (extra.highest_id_in_use > extra.highest_id_ever)
        extra.highest_id_ever = extra.highest_id_in_use;
}

void building_update_state(void) {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Building State Update");
    bool land_recalc = false;
    bool wall_recalc = false;
    bool road_recalc = false;
    bool water_routes_recalc = false;
    bool aqueduct_recalc = false;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = &g_all_buildings[i];
        if (b->state == BUILDING_STATE_CREATED)
            b->state = BUILDING_STATE_VALID;

        if (b->state != BUILDING_STATE_VALID || !b->house_size) {
            if (b->state == BUILDING_STATE_UNDO || b->state == BUILDING_STATE_DELETED_BY_PLAYER) {
                if (b->type == BUILDING_MUD_TOWER || b->type == BUILDING_MUD_GATEHOUSE) {
                    wall_recalc = true;
                    road_recalc = true;
                } else if (b->type == BUILDING_WATER_LIFT) {
                    aqueduct_recalc = true;
                } else if (b->type == BUILDING_GRANARY) {
                    road_recalc = true;
                } else if (b->type == BUILDING_FERRY) {
                    water_routes_recalc = true;
                }

                map_building_tiles_remove(i, b->tile);
                road_recalc = true; // always recalc underlying road tiles
                land_recalc = true;
                building_delete_UNSAFE(b);
            } else if (b->state == BUILDING_STATE_RUBBLE) {
                if (b->house_size)
                    city_population_remove_home_removed(b->house_population);
                building_delete_UNSAFE(b);
            } else if (b->state == BUILDING_STATE_DELETED_BY_GAME) {
                building_delete_UNSAFE(b);
            } 
        }
    }
    if (wall_recalc) {
        map_tiles_update_all_walls();
    }

    if (aqueduct_recalc) {
        map_tiles_update_all_aqueducts(0);
    }

    if (land_recalc) {
        map_routing_update_land();
    }

    if (road_recalc) {
        map_tiles_update_all_roads();
    }

    if (water_routes_recalc) {
        map_routing_update_ferry_routes();
    }
}

void building_update_desirability(void) {
    OZZY_PROFILER_SECTION("Game/Run/Tick/Building Update Desirability");
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = &g_all_buildings[i];
        if (b->state != BUILDING_STATE_VALID)
            continue;

        b->desirability = map_desirability_get_max(b->tile, b->size);
        if (b->is_adjacent_to_water)
            b->desirability += 10;

        switch (map_elevation_at(b->tile.grid_offset())) {
        case 0:
            break;
        case 1:
            b->desirability += 10;
            break;
        case 2:
            b->desirability += 12;
            break;
        case 3:
            b->desirability += 14;
            break;
        case 4:
            b->desirability += 16;
            break;
        default:
            b->desirability += 18;
            break;
        }
    }
}

int building_mothball_toggle(building* b) {
    if (b->state == BUILDING_STATE_VALID) {
        b->state = BUILDING_STATE_MOTHBALLED;
        b->num_workers = 0;
    } else if (b->state == BUILDING_STATE_MOTHBALLED) {
        b->state = BUILDING_STATE_VALID;
    }

    return b->state;
}

int building_mothball_set(building* b, int mothball) {
    if (mothball) {
        if (b->state == BUILDING_STATE_VALID) {
            b->state = BUILDING_STATE_MOTHBALLED;
            b->num_workers = 0;
        }
    } else if (b->state == BUILDING_STATE_MOTHBALLED) {
        b->state = BUILDING_STATE_VALID;
    }

    return b->state;
}

bool building_impl::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    int image_id = map_image_at(tile.grid_offset());
    building_draw_normal_anim(ctx, point, &base, tile, image_id, color_mask);
    if (base.has_plague) {
        ImageDraw::img_generic(ctx, image_id_from_group(GROUP_PLAGUE_SKULL), point.x + 18, point.y - 32, color_mask);
    }

    return false;
}

void building_impl::destroy_by_poof(bool clouds) {
    building_destroy_by_poof(&base, clouds);
}

bool resource_required_by_workshop(building* b, e_resource resource) {
    switch (resource) {
    case RESOURCE_CLAY: return (b->type == BUILDING_POTTERY_WORKSHOP || b->type == BUILDING_BRICKS_WORKSHOP);
    case RESOURCE_STRAW: return (b->type == BUILDING_BRICKS_WORKSHOP || b->type == BUILDING_CATTLE_RANCH);
    case RESOURCE_BARLEY: return b->type == BUILDING_BREWERY_WORKSHOP;
    case RESOURCE_REEDS: return b->type == BUILDING_PAPYRUS_WORKSHOP;
    case RESOURCE_FLAX: return b->type == BUILDING_WEAVER_WORKSHOP;
    case RESOURCE_GEMS: return b->type == BUILDING_JEWELS_WORKSHOP;
    case RESOURCE_COPPER: return b->type == BUILDING_WEAPONSMITH;
    case RESOURCE_TIMBER: return b->type == BUILDING_CHARIOTS_WORKSHOP;
    case RESOURCE_HENNA: return b->type == BUILDING_PAINT_WORKSHOP;
    case RESOURCE_OIL: return b->type == BUILDING_LAMP_WORKSHOP;
    }
    return false;
}

void building_impl::static_params::load(archive arch) {
    labor_category = arch.r_type<e_labor_category>("labor_category");
    output_resource = arch.r_type<e_resource>("output_resource");
    meta_id = arch.r_string("meta_id");
    anim.load(arch);
}


// void building_load_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever) {

// iob->bind(BIND_SIGNATURE_INT32, &//    extra.created_sequence);
// iob->bind(BIND_SIGNATURE_INT32, &//    extra.incorrect_houses);
// iob->bind(BIND_SIGNATURE_INT32, &//    extra.unfixable_houses);
// }

static void read_type_data(io_buffer *iob, building *b, size_t version) {
    if (building_is_house(b->type)) {
        for (e_resource e = RESOURCE_NONE; e < RESOURCES_FOODS_MAX; ++e) {
            iob->bind(BIND_SIGNATURE_INT16, &b->data.house.foods[e]);
        }

        for (int i = 0; i < 4; i++) {
            int good_n = ALLOWED_FOODS(i);
            b->data.house.inventory[i] = b->data.house.inventory[good_n];
            iob->bind(BIND_SIGNATURE_INT16, &b->data.house.inventory[i + 4]);
        }

        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.juggler);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.bandstand_juggler);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.bandstand_musician);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.senet_player);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.magistrate);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.hippodrome);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.school);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.library);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.academy);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.apothecary);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.dentist);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.mortuary);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.physician);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.temple_osiris);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.temple_ra);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.temple_ptah);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.temple_seth);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.temple_bast);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.no_space_to_expand);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.num_foods);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.entertainment);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.education);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.health);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.num_gods);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.devolve_delay);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.evolve_text_id);

        if (version <= 160) { b->data.house.shrine_access = 0; }         else { iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.shrine_access); }

        if (version <= 162) { b->data.house.bazaar_access = 0; }         else { iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.bazaar_access); }

        if (version <= 163) { b->data.house.water_supply = 0; }         else { iob->bind(BIND_SIGNATURE_UINT8, &b->data.house.water_supply); }

    } else if (b->type == BUILDING_BAZAAR) {
        iob->bind____skip(2);
        //            iob->bind____skip(8);
        iob->bind(BIND_SIGNATURE_INT16, &b->data.market.pottery_demand);
        iob->bind(BIND_SIGNATURE_INT16, &b->data.market.furniture_demand);
        iob->bind(BIND_SIGNATURE_INT16, &b->data.market.oil_demand);
        iob->bind(BIND_SIGNATURE_INT16, &b->data.market.wine_demand);
        for (int i = 0; i < INVENTORY_MAX; i++) {
            iob->bind(BIND_SIGNATURE_INT32, &b->data.market.inventory[i]);
        }
        //            iob->bind____skip(6);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.market.fetch_inventory_id);
        iob->bind____skip(7);
        //            iob->bind____skip(6);
        //            iob->bind(BIND_SIGNATURE_UINT8, &b->data.market.fetch_inventory_id);
        //            iob->bind____skip(9);
    } else if (b->type == BUILDING_GRANARY) {
        iob->bind____skip(2);
        iob->bind____skip(2);

        for (int i = 0; i < RESOURCES_MAX; i++) {
            iob->bind(BIND_SIGNATURE_INT16, &b->data.granary.resource_stored[i]);
            b->data.granary.resource_stored[i] = (b->data.granary.resource_stored[i] / 100) * 100; // todo
        }
        iob->bind____skip(6);

    } else if (b->type == BUILDING_DOCK) {
        iob->bind(BIND_SIGNATURE_INT16, &b->data.dock.queued_docker_id);
        iob->bind(BIND_SIGNATURE_INT32, &b->data.dock.dock_tiles[0]);
        iob->bind(BIND_SIGNATURE_INT32, &b->data.dock.dock_tiles[1]);
        iob->bind____skip(17);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.dock.num_ships);
        iob->bind____skip(2);
        iob->bind(BIND_SIGNATURE_INT8, &b->data.dock.orientation);
        iob->bind____skip(3);
        for (int i = 0; i < 3; i++) {
            iob->bind(BIND_SIGNATURE_INT16, &b->data.dock.docker_ids[i]);
        }
        iob->bind(BIND_SIGNATURE_INT16, &b->data.dock.trade_ship_id);

    } else if (building_is_industry_type(b)) {
        iob->bind(BIND_SIGNATURE_INT16, &b->data.industry.ready_production);
        iob->bind(BIND_SIGNATURE_INT16, &b->data.industry.progress);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.spawned_worker_this_month);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.max_gatheres);
        for (int i = 0; i < 10; i++) {
            iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.unk_b[i]);
        }
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.has_fish);
        for (int i = 0; i < 14; i++) {
            iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.unk_c[i]);
        }
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.blessing_days_left);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.orientation);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.has_raw_materials);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.second_material_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.curse_days_left);
        iob->bind(BIND_SIGNATURE_UINT16, &b->data.industry.stored_amount_second);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.first_material_id);
        for (int i = 0; i < 3; i++) {
            iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.unk_6[i]);
        }
        iob->bind(BIND_SIGNATURE_INT16, &b->data.industry.fishing_boat_id);

        for (int i = 0; i < 40; i++) {
            iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.unk_40[i]);
        }
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.labor_state);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.labor_days_left);
        for (int i = 0; i < 10; i++) {
            iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.unk_12[i]);
        }
        iob->bind(BIND_SIGNATURE_UINT16, &b->data.industry.work_camp_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.worker_id);

    } else if (building_is_statue(b->type) || building_is_large_temple(b->type) || building_is_monument(b->type)) {
        iob->bind____skip(38);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.monuments.orientation);
        for (int i = 0; i < 5; i++) {
            iob->bind(BIND_SIGNATURE_UINT16, &b->data.monuments.workers[i]);
        }
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.monuments.phase);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.monuments.statue_offset);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.monuments.temple_complex_attachments);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.monuments.variant);

        for (int i = 0; i < RESOURCES_MAX; i++) {
            iob->bind(BIND_SIGNATURE_UINT8, &b->data.monuments.resources_pct[i]);
        }
    } else if (b->type == BUILDING_WATER_LIFT || b->type == BUILDING_FERRY) {
        iob->bind____skip(88);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.industry.orientation);

    } else if (building_is_guild(b->type)) {
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.guild.max_workers);
    } else {
        iob->bind____skip(26);
        iob->bind____skip(58);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.entertainment.num_shows);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.entertainment.days1);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.entertainment.days2);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.entertainment.days3_or_play);
        iob->bind____skip(20);
        iob->bind(BIND_SIGNATURE_UINT32, &b->data.entertainment.latched_venue_main_grid_offset);
        iob->bind(BIND_SIGNATURE_UINT32, &b->data.entertainment.latched_venue_add_grid_offset);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.entertainment.orientation);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.entertainment.ent_reserved_u8);
        iob->bind____skip(6);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.entertainment.consume_material_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->data.entertainment.spawned_entertainer_this_month);
        iob->bind(BIND_SIGNATURE_UINT32, &b->data.entertainment.booth_corner_grid_offset);
    }
}

io_buffer* iob_buildings = new io_buffer([](io_buffer* iob, size_t version) {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        //        building_state_load_from_buffer(buf, &all_buildings[i]);
        auto b = &g_all_buildings[i];
        int sind = (int)iob->get_offset();
        if (sind == 640) {
            int a = 2134;
        }

        iob->bind(BIND_SIGNATURE_UINT8, &b->state);
        iob->bind(BIND_SIGNATURE_UINT8, &b->faction_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->reserved_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->size);
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_is_merged);
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_size);
        iob->bind(BIND_SIGNATURE_INT16, b->tile.private_access(_X));
        iob->bind(BIND_SIGNATURE_INT16, b->tile.private_access(_Y));
        iob->bind____skip(2);
        iob->bind(BIND_SIGNATURE_INT32, b->tile.private_access(_GRID_OFFSET));
        iob->bind(BIND_SIGNATURE_INT16, &b->type);
        iob->bind(BIND_SIGNATURE_INT16, &b->subtype.house_level); // which union field we use does not matter
        iob->bind(BIND_SIGNATURE_UINT16, &b->road_network_id);
        iob->bind(BIND_SIGNATURE_UINT16, &b->creation_sequence_index);
        iob->bind(BIND_SIGNATURE_INT16, &b->houses_covered);
        iob->bind(BIND_SIGNATURE_INT16, &b->percentage_houses_covered);
        iob->bind(BIND_SIGNATURE_INT16, &b->house_population);
        iob->bind(BIND_SIGNATURE_INT16, &b->house_population_room);
        iob->bind(BIND_SIGNATURE_INT16, &b->distance_from_entry);
        iob->bind(BIND_SIGNATURE_INT16, &b->house_highest_population);

        iob->bind(BIND_SIGNATURE_INT16, &b->house_unreachable_ticks);
        iob->bind(BIND_SIGNATURE_UINT16, b->road_access.private_access(_X));
        iob->bind(BIND_SIGNATURE_UINT16, b->road_access.private_access(_Y));
        //        b->set_figure(0, buf->read_u16());
        //        b->set_figure(1, buf->read_u16());
        //        b->set_figure(2, buf->read_u16());
        //        b->set_figure(3, buf->read_u16());
        b->bind_iob_figures(iob);

        iob->bind(BIND_SIGNATURE_INT16, &b->figure_spawn_delay);
        iob->bind(BIND_SIGNATURE_UINT8, &b->figure_roam_direction);
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_water_access);

        iob->bind(BIND_SIGNATURE_UINT8, &b->common_health);
        iob->bind(BIND_SIGNATURE_UINT8, &b->malaria_risk);
        iob->bind(BIND_SIGNATURE_INT16, &b->prev_part_building_id);
        iob->bind(BIND_SIGNATURE_INT16, &b->next_part_building_id);
        iob->bind(BIND_SIGNATURE_UINT16, &b->stored_full_amount);
        iob->bind(BIND_SIGNATURE_UINT8, &b->disease_days);
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_well_access);

        iob->bind(BIND_SIGNATURE_INT16, &b->num_workers);
        iob->bind(BIND_SIGNATURE_UINT8, &b->labor_category); // FF
        iob->bind(BIND_SIGNATURE_UINT8, &b->output_resource_first_id);
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_road_access);
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_criminal_active);

        iob->bind(BIND_SIGNATURE_INT16, &b->damage_risk);
        iob->bind(BIND_SIGNATURE_INT16, &b->fire_risk);
        iob->bind(BIND_SIGNATURE_INT16, &b->fire_duration);
        iob->bind(BIND_SIGNATURE_UINT8, &b->fire_proof);

        iob->bind(BIND_SIGNATURE_UINT8, &b->map_random_7bit); // 20 (workcamp 1)
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_tax_coverage);
        iob->bind(BIND_SIGNATURE_UINT8, &b->health_proof);
        iob->bind(BIND_SIGNATURE_INT16, &b->formation_id); 

        read_type_data(iob, b, version); // 42 bytes for C3, 102 for PH

        int currind = iob->get_offset() - sind;
        iob->bind____skip(184 - currind);

        iob->bind(BIND_SIGNATURE_INT32, &b->tax_income_or_storage);
        iob->bind(BIND_SIGNATURE_UINT8, &b->house_days_without_food);
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_plague); // 1

        iob->bind(BIND_SIGNATURE_INT8, &b->desirability);
        iob->bind(BIND_SIGNATURE_UINT8, &b->is_deleted);
        iob->bind(BIND_SIGNATURE_UINT8, &b->is_adjacent_to_water);

        iob->bind(BIND_SIGNATURE_UINT8, &b->storage_id);
        iob->bind(BIND_SIGNATURE_INT8, &b->sentiment.house_happiness); // which union field we use does not matter // 90 for house, 50 for wells
        iob->bind(BIND_SIGNATURE_UINT8, &b->show_on_problem_overlay); // 1
        iob->bind(BIND_SIGNATURE_UINT16, &b->deben_storage); // 2
        iob->bind(BIND_SIGNATURE_UINT8, &b->has_open_water_access); // 1
        iob->bind(BIND_SIGNATURE_UINT8, &b->output_resource_second_id); // 1
        iob->bind(BIND_SIGNATURE_UINT8, &b->output_resource_second_rate); // 1

        // 63 additional bytes
        iob->bind____skip(63); // temp for debugging
                               //            assert(iob->get_offset() - sind == 264);
        g_all_buildings[i].id = i;

        if (version <= 164) { 
            b->common_health = 100;
            b->malaria_risk = 0;
            b->disease_days = 0;
            b->health_proof = 0;
        }
    }
    building_extra_data.created_sequence = 0;
});

io_buffer* iob_building_highest_id = new io_buffer([](io_buffer* iob, size_t version) {
    iob->bind(BIND_SIGNATURE_INT32, &building_extra_data.highest_id_in_use);
});

io_buffer* iob_building_highest_id_ever = new io_buffer([](io_buffer* iob, size_t version) {
    iob->bind(BIND_SIGNATURE_INT32, &building_extra_data.highest_id_ever);
    iob->bind____skip(4);
    //    highest_id_ever->skip(4);
});