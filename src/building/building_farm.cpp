#include "building_farm.h"

#include "building/building.h"
#include "city/object_info.h"
#include "city/resource.h"
#include "core/calc.h"
#include "game/resource.h"
#include "graphics/elements/panel.h"
#include "graphics/elements/lang_text.h"
#include "graphics/view/view.h"
#include "graphics/text.h"
#include "graphics/boilerplate.h"
#include "grid/floodplain.h"
#include "io/gamefiles/lang.h"
#include "config/config.h"
#include "window/building/common.h"
#include "sound/sound_building.h"
#include "game/game.h"

static void building_farm_draw_info(object_info &c, int help_id, const char* type, int group_id, e_resource resource) {
    painter ctx = game.painter();

    c.help_id = help_id;
    window_building_play_sound(&c, snd::get_building_info_sound(type));

    outer_panel_draw(c.offset.x, c.offset.y, c.width_blocks, c.height_blocks);
    ctx.draw(sprite_resource_icon(resource), c.offset + vec2i{10, 10});
    lang_text_draw_centered(group_id, 0, c.offset.x, c.offset.y + 10, 16 * c.width_blocks, FONT_LARGE_BLACK_ON_LIGHT);

    building* b = building_get(c.building_id);
    //int pct_grown = calc_percentage(b->data.industry.progress, 200);
    int pct_grown = calc_percentage(b->data.industry.progress, 2000);
    int width = lang_text_draw(group_id, 2, c.offset.x + 32, c.offset.y + 44, FONT_NORMAL_BLACK_ON_LIGHT);
    width += text_draw_percentage(pct_grown, c.offset.x + 32 + width, c.offset.y + 44, FONT_NORMAL_BLACK_ON_LIGHT);
    width += lang_text_draw(group_id, 3, c.offset.x + 32 + width, c.offset.y + 44, FONT_NORMAL_BLACK_ON_LIGHT);

    // fertility
    int pct_fertility = map_get_fertility_for_farm(b->tile.grid_offset());
    width += lang_text_draw(group_id, 12, c.offset.x + 32 + width, c.offset.y + 44, FONT_NORMAL_BLACK_ON_LIGHT);
    width += text_draw_percentage(pct_fertility, c.offset.x + 32 + width, c.offset.y + 44, FONT_NORMAL_BLACK_ON_LIGHT);
    lang_text_draw(group_id, 13, c.offset.x + 32 + width, c.offset.y + 44, FONT_NORMAL_BLACK_ON_LIGHT);

    if (!c.has_road_access)
        window_building_draw_description_at(c, 70, 69, 25);
    else if (city_resource_is_mothballed(resource))
        window_building_draw_description_at(c, 70, group_id, 4);
    else if (b->data.industry.curse_days_left > 4)
        window_building_draw_description_at(c, 70, group_id, 11);
    else if (b->num_workers <= 0)
        window_building_draw_description_at(c, 70, group_id, 5);
    else if (c.worker_percentage >= 100)
        window_building_draw_description_at(c, 70, group_id, 6);
    else if (c.worker_percentage >= 75)
        window_building_draw_description_at(c, 70, group_id, 7);
    else if (c.worker_percentage >= 50)
        window_building_draw_description_at(c, 70, group_id, 8);
    else if (c.worker_percentage >= 25)
        window_building_draw_description_at(c, 70, group_id, 9);
    else
        window_building_draw_description_at(c, 70, group_id, 10);

    inner_panel_draw(c.offset.x + 16, c.offset.y + 136, c.width_blocks - 2, 4);
    if (building_is_floodplain_farm(*b)) {
        window_building_draw_employment_flood_farm(&c, 142);

        // next flood info
        int month_id = 8; // TODO: fetch flood info
        width = lang_text_draw(177, 2, c.offset.x + 32, c.offset.y + 16 * c.height_blocks - 136, FONT_NORMAL_BLACK_ON_LIGHT);
        lang_text_draw(160, month_id, c.offset.x + 32 + width, c.offset.y + 16 * c.height_blocks - 136, FONT_NORMAL_BLACK_ON_LIGHT);

        // irrigated?
        int is_not_irrigated = 0; // TODO: fetch irrigation info
        lang_text_draw(177, is_not_irrigated, c.offset.x + 32, c.offset.y + 16 * c.height_blocks - 120,FONT_NORMAL_BLACK_ON_LIGHT);

        window_building_draw_description_at(c, 16 * c.height_blocks - 96, group_id, 1);
    } else {
        window_building_draw_employment(&c, 142);
        window_building_draw_description_at(c, 16 * c.height_blocks - 136, group_id, 1);
    }
}

void building_farm_draw_info(object_info& c) {
    building *b = building_get(c.building_id);

    int farm_group_id = 0;
    e_resource output_resource = RESOURCE_NONE;
    switch (b->type) {
    case BUILDING_BARLEY_FARM:
        farm_group_id = 181;
        output_resource = RESOURCE_BARLEY;
        building_farm_draw_info(c, 89, "barley_farm", farm_group_id, output_resource);
        break;

    case BUILDING_FLAX_FARM:
        farm_group_id = 115;
        output_resource = RESOURCE_FLAX;
        building_farm_draw_info(c, 90, "flag_farm", farm_group_id, output_resource);
        break;

    case BUILDING_GRAIN_FARM:
        farm_group_id = 112;
        output_resource = RESOURCE_GRAIN;
        building_farm_draw_info(c, 90, "grain_farm", farm_group_id, output_resource);
        break;

    case BUILDING_LETTUCE_FARM:
        farm_group_id = 113;
        output_resource = RESOURCE_LETTUCE;
        building_farm_draw_info(c, 91, "lettuce_farm", farm_group_id, output_resource);
        break;

    case BUILDING_POMEGRANATES_FARM:
        farm_group_id = 114;
        output_resource = RESOURCE_POMEGRANATES;
        building_farm_draw_info(c, 91, "pomegranades_farm", farm_group_id, output_resource);
        break;

    case BUILDING_CHICKPEAS_FARM:
        farm_group_id = 182;
        output_resource = RESOURCE_CHICKPEAS;
        building_farm_draw_info(c, 90, "chickpeas_farm", farm_group_id, output_resource);
        break;

    case BUILDING_FIGS_FARM:
        building_farm_draw_info(c, 90, "figs_farm", 183, RESOURCE_FIGS);;
        break;

    case BUILDING_HENNA_FARM:
        building_farm_draw_info(c, 90, "henna_farm", 306, RESOURCE_HENNA);
        break;
    }
}