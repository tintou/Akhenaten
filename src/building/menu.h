#pragma once

#include "building/building_type.h"
#include "city/constants.h"
#include "city/gods.h"

enum e_build_menu {
    BUILD_MENU_VACANT_HOUSE = 0,
    BUILD_MENU_CLEAR_LAND = 1,
    BUILD_MENU_ROAD = 2,
    BUILD_MENU_WATER = 3,
    BUILD_MENU_WATER_CROSSINGS = 3,
    BUILD_MENU_HEALTH = 4,
    BUILD_MENU_RELIGION = 5,
    BUILD_MENU_EDUCATION = 6,
    BUILD_MENU_ENTERTAINMENT = 7,
    BUILD_MENU_ADMINISTRATION = 8,
    BUILD_MENU_ENGINEERING = 9,
    BUILD_MENU_BEAUTIFICATION = 9,
    BUILD_MENU_SECURITY = 10,
    BUILD_MENU_INDUSTRY = 11,

    BUILD_MENU_FARMS = 12,
    BUILD_MENU_RAW_MATERIALS = 13,
    BUILD_MENU_WORKSHOPS = 14,
    BUILD_MENU_GUILDS = 14,
    BUILD_MENU_SMALL_TEMPLES = 15,
    BUILD_MENU_TEMPLES = 15,
    BUILD_MENU_LARGE_TEMPLES = 16,
    BUILD_MENU_TEMPLE_COMPLEX = 16,
    BUILD_MENU_FORTS = 17,
    //
    BUILD_MENU_FOOD = 18,
    BUILD_MENU_DISTRIBUTION = 19,
    BUILD_MENU_SHRINES = 20,
    BUILD_MENU_MONUMENTS = 21,
    BUILD_MENU_DEFENCES = 22,
    //
    BUILD_MENU_MAX
};

enum e_buildset_tutorial {
    BUILDSET_NORMAL,
    //
    BUILDSET_TUT1_START,
    BUILDSET_TUT1_FIRE,
    BUILDSET_TUT1_CRIME,
    BUILDSET_TUT1_FOOD,
    BUILDSET_TUT1_WATER,
    BUILDSET_TUT1_COLLAPSE,

    BUILDSET_TUT2_START,
    BUILDSET_TUT2_GODS,
    BUILDSET_TUT2_ENTERTAINMENT,

    BUILDSET_TUT3_START,
    BUILDSET_TUT3_INDUSTRY,
    BUILDSET_TUT3_HEALTH,
    BUILDSET_TUT3_GARDENS,

    BUILDSET_TUT4_START,
    BUILDSET_TUT4_FINANCE,

    BUILDSET_TUT5_START,
    BUILDSET_TUT5_EDUCATION,
    BUILDSET_TUT5_TRADING,
    BUILDSET_TUT5_MONUMENTS,

    BUILDSET_TUT6_START,
    BUILDSET_TUT6_IRRIGATION,
    BUILDSET_TUT6_EDUCATION,
    BUILDSET_TUT6_GUILDS,

    BUILDSET_TUT7_START,

    BUILDSET_TUT8_START,
};

int building_menu_is_building_enabled(int type);
void building_menu_toggle_building(int type, bool enabled = true);

void building_menu_enable_all(void);

void building_menu_update(int build_set);

int building_menu_count_items(int submenu);

int building_menu_next_index(int submenu, int current_index);

e_building_type building_menu_type(int submenu, int item);

void building_menu_update_monuments();
void building_menu_update_temple_complexes();
void building_menu_update_gods_available(e_god god, bool available);

bool building_menu_has_changed();
void building_menu_invalidate();