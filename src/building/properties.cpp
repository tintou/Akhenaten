#include "properties.h"
#include "core/game_environment.h"
#include "graphics/image_groups.h"

constexpr int BUILD_MAX = 300;

static building_properties properties[400] = {
    // PHARAOH
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1, 0, 0, 0}, // road
    {1, 0, 0, 0}, // wall (unused)
    {0, 0, 0, 0},
    {1, 0, 0, 0},                            // irrigation ditch
    {0, 0, 0, 0},                            // clear land
    {1, 0, GROUP_BUILDING_HOUSE_VACANT_LOT}, // houses vvvv
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {2, 0, 0, 0},
    {2, 0, 0, 0},
    {2, 0, 0, 0},
    {2, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {4, 0, 0, 0},
    {4, 0, 0, 0},                          // houses ^^^^
    {1, 0, 0, 0},                          // bandstand
    {1, 0, 0, 0},                          // booth
    {4, 0, GROUP_BUILDING_SENET_HOUSE},    // senet
    {2, 0, 0, 0},                          // pavillion
    {3, 0, 0, 0},   // conservatory
    {4, 0, GROUP_BUILDING_DANCE_SCHOOL},   // dance school
    {2, 0, 0, 0}, // juggler school
    {3, 0, 0, 0},                          // unused (senet master)
    {1, 1, GROUP_TERRAIN_PLAZA},           // plaza
    {1, 1, GROUP_TERRAIN_GARDEN},          // gardens
    {3, 1, 0, 0},                          // charioteers
    {1, 1, 0, 0},
    {2, 1, 0, 0},
    {3, 1, 0, 0},                      // statues ^^^
    {3, 1, GROUP_BUILDING_FORT},       // archers
    {3, 1, GROUP_BUILDING_FORT},       // infantry
    {1, 0, 0, 0}, // apothecary
    {2, 0, GROUP_BUILDING_MORTUARY},   // mortuary
    {2, 0, 0, 0},
    {1, 0, GROUP_BUILDING_DENTIST},        // dentist
    {3, 0, 0, 0},                          // unused (distribution center)
    {2, 0, 0, 0},         // school
    {3, 0, 0, 0},                          // water crossings
    {3, 0, GROUP_BUILDING_LIBRARY},        // library
    {4, 1, GROUP_BUILDING_FORT, 1},        // fort yard
    {1, 0, GROUP_BUILDING_POLICE_STATION}, // police
    {3, 1, 0, 0},
    {3, 1, GROUP_BUILDING_FORT}, // fort (main)
    {2, 1, 0, 0},
    {2, 1, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},   // temples ^^^^
    {3, 0, 0, 0},                         // oz
    {3, 0, 0, 0},                         // ra
    {3, 0, 0, 0},                         // ptah
    {3, 0, 0, 0},                         // seth
    {3, 0, 0, 0},                         // temple complex ^^^^
    {2, 0, 0, 0},        // bazaar
    {4, 0, 0, 0},       // granary
    {1, 1, 0, 0},  // storageyard (hut)
    {1, 1, 0, 0},                         // storageyard (space tile)
    {3, 0, GROUP_BUILDING_SHIPYARD},      // shipwright
    {3, 0, GROUP_BUILDING_DOCK},          // dock
    {2, 0, GROUP_BUILDING_FISHING_WHARF}, // wharf (fish)
    {3, 0, 0, 0},
    {4, 0, GROUP_BUILDING_GOVERNORS_VILLA},
    {5, 0, GROUP_BUILDING_GOVERNORS_PALACE}, // mansions ^^^
    {2, 1, 0, 0},
    {1, 1, 0, 0}, // architects
    {1, 1, 0, 0},                          // bridge
    {1, 1, 0, 0},
    {0, 0, 0, 0}, // senate (unused?)
    {5, 0, 0, 0}, // senate2 (unused?)
    {2, 0, 0, 0},
    {2, 0, 0, 0}, // tax collectors ^^
    {1, 1, 0, 0},
    {2, 1, 0, 0},
    {2, 1, GROUP_BUILDING_WATER_LIFT}, // ??????? water lift ???????
    {1, 1, 0, 0},
    {1, 1, 0, 0}, // well
    {1, 1, 0, 0},
    {4, 0, GROUP_BUILDING_MILITARY_ACADEMY}, // academy
    {3, 0, 0, 0},         // recruiter
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {2, 0, 0, 0},
    {1, 1, 0, 0}, // burning ruin
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},
    {3, 0, 0, 0},                            // farms ^^^
    {2, 0, 0, 0},     // stone
    {2, 0, GROUP_BUILDING_LIMESTONE_QUARRY}, // limestone
    {2, 0, GROUP_BUILDING_TIMBER_YARD},      // wood
    {2, 0, 0, 0},         // clay
    {2, 0, 0, 0},    // beer
    {2, 0, GROUP_BUILDING_LINEN_WORKSHOP},   // linen
    {2, 0, 0, 0}, // weapons
    {2, 0, GROUP_BUILDING_JEWELS_WORKSHOP},  // jewels
    {2, 0, 0, 0}, // pottery

    {2, 0, 0, 0}, // hunters
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {4, 0, 0, 0},
    {2, 1, GROUP_BUILDING_FERRY}, // ferry
    {2, 0, 0, 0},
    {1, 1, GROUP_BUILDING_ROADBLOCK}, // roadblock
    {0, 0, 0, 0},
    {1, 0, GROUP_BUILDING_SHRINE_OSIRIS},
    {1, 0, GROUP_BUILDING_SHRINE_RA},
    {1, 0, GROUP_BUILDING_SHRINE_PTAH},
    {1, 0, GROUP_BUILDING_SHRINE_SETH},
    {1, 0, GROUP_BUILDING_SHRINE_BAST}, // shrines ^^^^
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {2, 0, 0, 0},     // gold
    {2, 0, 0, 0}, // gemstone
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1, 0, 0, 0}, // firehouse
    {0, 0, 0, 0},
    {1, 1, 0, 0}, // wall
    {0, 0, 0, 0},
    {1, 1, 0, 0}, // gatehouse
    {0, 0, 0, 0},
    {2, 1, GROUP_BUILDING_TOWER}, // tower
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1, 1, 0, 0},
    {2, 0, GROUP_BUILDING_GUILD_CARPENTERS},
    {2, 0, 0, 0},
    {2, 0, GROUP_BUILDING_GUILD_STONEMASONS}, // guilds ^^^
    {2, 1, 0, 0, IMG_WATER_SUPPLY},      // water supply
    {2, 1, GROUP_BUILDING_TRANSPORT_WHARF},   // wharf (transport)
    {3, 1, GROUP_BUILDING_WARSHIP_WHARF},     // wharf (warship)
    {-1, 1, 0, 0},                            // pyramid
    {3, 0, 0, 0},        // courthouse
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {4, 0, 0, 0},
    {5, 0, GROUP_BUILDING_TOWN_PALACE},
    {6, 0, GROUP_BUILDING_CITY_PALACE}, // town palace ^^^
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {3, 0, 0, 0},
    {2, 0, 0, 0}, // reeds
    {3, 1, 0, 0},                           // figs farm
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {2, 0, 0, 0}, // work camp
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1, 1, 0, 0},                              // gatehouse (2)
    {2, 0, 0, 0},   // papyrus
    {2, 0, 0, 0, 0}, // bricks
    {4, 0, 0, 0},                              // chariots
    {2, 0, 0, 0},       // physician
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {5, 1, 0, 0},  // festival square
    {-1, 1, 0, 0}, // sphynx
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {2, 0, GROUP_BUILDING_GRANITE_QUARY, 0}, // granite
    {2, 0, 0, 0},   // copper
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {2, 0, 0, 0}, // sandstone
    {-1, 1, 0, 0},                            // mausoleum
    {0, 0, 0, 0},
    {3, 0, 0, 0},  // henna
    {-1, 1, 0, 0}, // alexandria library
};

building_properties ROADBLOCK_PROPERTIES = {1, 1, 10000, 0};

const building_properties dummy_property = {0, 0, 0, 0};

void building_properties_init() {
    properties[BUILDING_WATER_SUPPLY] = {2, 1, 0, 0, 0, IMG_WATER_SUPPLY};
    properties[BUILDING_WELL] = {1, 1, 0, 0, 0, IMG_WELL};
    properties[BUILDING_CATTLE_RANCH] = {3, 0, 0, 0, 0, IMG_CATTLE_RANCH};
    properties[BUILDING_BRICKS_WORKSHOP] = {2, 0, 0, 0, 0, IMG_BRICKS_WORKSHOP};
    properties[BUILDING_GOLD_MINE] = {2, 0, 0, 0, 0, IMG_GOLD_MINE};
    properties[BUILDING_SANDSTONE_QUARRY] = {2, 0, 0, 0, 0, IMG_SANDSTONE_QUARRY};
    properties[BUILDING_STONE_QUARRY] = {2, 0, 0, 0, 0, IMG_PLAINSTONE_QUARRY};
    properties[BUILDING_GRANARY] = {4, 0, 0, 0, 0, IMG_GRANARY};
    properties[BUILDING_COPPER_MINE] = {2, 0, 0, 0, 0, IMG_COPPER_MINE};
    properties[BUILDING_CONSERVATORY] = {3, 0, 0, 0, 0, IMG_CONSERVATORY};
    properties[BUILDING_PHYSICIAN] = {2, 0, 0, 0, 0, IMG_PHYSICIAN};
    properties[BUILDING_RECRUITER] = {3, 0, 0, 0, 0, IMG_BARRACKS};
    properties[BUILDING_SCRIBAL_SCHOOL] = {2, 0, 0, 0, 0, IMG_SCRIBAL_SCHOOL};
    properties[BUILDING_TAX_COLLECTOR] = {2, 0, 0, 0, 0, IMG_TAX_COLLECTOR_BUILDING};
    properties[BUILDING_TAX_COLLECTOR_UPGRADED] = {2, 0, 0, 0, 0, IMG_TAX_COLLECTOR_BUILDING_UP};
    properties[BUILDING_POTTERY_WORKSHOP] = {2, 0, 0, 0, 0, IMG_POTTERY_WORKSHOP};
    properties[BUILDING_BREWERY_WORKSHOP] = {2, 0, 0, 0, 0, ANIM_BREWERY_WORKSHOP};
    properties[BUILDING_SMALL_MASTABA] = {2, 0, 0, 0, 0, IMG_SMALL_MASTABA};
    properties[BUILDING_SMALL_MASTABA_SIDE] = {2, 0, 0, 0, 0, IMG_SMALL_MASTABA};
    properties[BUILDING_SMALL_MASTABA_WALL] = {2, 0, 0, 0, 0, IMG_SMALL_MASTABA};
    properties[BUILDING_SMALL_MASTABA_ENTRANCE] = {2, 0, 0, 0, 0, IMG_SMALL_MASTABA};
    properties[BUILDING_BAZAAR] = {2, 0, 0, 0, 0, IMG_BAZAAR};
    properties[BUILDING_BRICKLAYERS_GUILD] = {2, 0, 0, 0, 0, IMG_BRICKLAYERS_GUILD};
    properties[BUILDING_FIREHOUSE] = {1, 0, 0, 0, 0, IMG_BUILDING_FIREHOUSE};
    properties[BUILDING_APOTHECARY] = {1, 0, 0, 0, 0, IMG_BUILDING_APOTHECARY};
    properties[BUILDING_PERSONAL_MANSION] = {3, 0, 0, 0, 0, IMG_PERSONAL_MANSION};
    properties[BUILDING_VILLAGE_PALACE] = {4, 0, 0, 0, 0, IMG_VILLAGE_PALACE};
    properties[BUILDING_JUGGLER_SCHOOL] = {2, 0, 0, 0, 0, IMG_JUGGLER_SCHOOL};
    properties[BUILDING_HUNTING_LODGE] = {2, 0, 0, 0, 0, IMG_HUNTING_LODGE};
    properties[BUILDING_CLAY_PIT] = {2, 0, 0, 0, 0, IMG_CLAY_PIT};
    properties[BUILDING_GEMSTONE_MINE] = {2, 0, 0, 0, 0, IMG_GEMSTONE_MINE};
    properties[BUILDING_TEMPLE_OSIRIS] = {3, 0, 0, 0, 0, IMG_TEMPLE_OSIRIS};
    properties[BUILDING_TEMPLE_RA] = {3, 0, 0, 0, 0, IMG_TEMPLE_RA};
    properties[BUILDING_TEMPLE_PTAH] = {3, 0, 0, 0, 0, IMG_TEMPLE_PTAH};
    properties[BUILDING_TEMPLE_SETH] = {3, 0, 0, 0, 0, IMG_TEMPLE_SETH};
    properties[BUILDING_TEMPLE_BAST] = {3, 0, 0, 0, 0, IMG_TEMPLE_BAST};
    properties[BUILDING_COURTHOUSE] = {3, 0, 0, 0, 0, IMG_COURTHOUSE};
    properties[BUILDING_WEAPONSMITH] = {2, 0, 0, 0, 0, IMG_WEAPONSMITH};
    properties[BUILDING_ARCHITECT_POST] = {1, 0, 0, 0, 0, IMG_ARCHITECT_POST};
    properties[BUILDING_STORAGE_YARD] = {1, 0, 0, 0, 0, IMG_STORAGE_YARD};
    properties[BUILDING_WORK_CAMP] = {2, 0, 0, 0, 0, IMG_WORKCAMP};
    properties[BUILDING_REED_GATHERER] = {2, 0, 0, 0, 0, IMG_BUILDING_REED_GATHERER};
    properties[BUILDING_PAPYRUS_WORKSHOP] = {2, 0, 0, 0, 0, IMG_PAPYRUS_WORKSHOP};
}

const building_properties* building_properties_for_type(e_building_type type) {
    if (type >= BUILD_MAX) {
        return &dummy_property;
    }

    auto p = &properties[type];
    return p;
}
