#include "common.h"

#include "building/building.h"
#include "building/model.h"
#include "city/labor.h"
#include "city/population.h"
#include "graphics/image.h"
#include "graphics/graphics.h"
#include "graphics/elements/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/view/view.h"
#include "sound/speech.h"
#include "game/game.h"

void window_building_set_possible_position(int* x_offset, int* y_offset, int width_blocks, int height_blocks) {
    int dialog_width = 16 * width_blocks;
    int dialog_height = 16 * height_blocks;
    vec2i view_pos, view_size;
    view_data_t viewport = city_view_viewport();
    city_view_get_viewport(viewport, view_pos, view_size);
    view_size.x -= MARGIN_POSITION;

    if (*y_offset + dialog_height > screen_height() - MARGIN_POSITION) {
        *y_offset -= dialog_height;
    }

    *y_offset = (*y_offset < MIN_Y_POSITION) ? MIN_Y_POSITION : *y_offset;
    
    if (*x_offset + dialog_width > view_size.x) {
        *x_offset = view_size.x - dialog_width;
    }
}
int window_building_get_vertical_offset(object_info* c, int new_window_height) {
    new_window_height = new_window_height * 16;
    int old_window_height = c->height_blocks * 16;
    c->height_blocks_submenu = new_window_height / 16;

    int center = (old_window_height / 2) + c->offset.y;
    int new_window_y = center - (new_window_height / 2);

    if (new_window_y < MIN_Y_POSITION) {
        new_window_y = MIN_Y_POSITION;
    } else {
        int height = screen_height() - MARGIN_POSITION;

        if (new_window_y + new_window_height > height)
            new_window_y = height - new_window_height;
    }

    c->y_offset_submenu = new_window_y;
    return new_window_y;
}

int get_employment_info_text_id(object_info* c, building* b, int y_offset, int consider_house_covering) {
    int text_id;
    if (b->num_workers >= model_get_building(b->type)->laborers)
        text_id = 0;
    else if (city_population() <= 0)
        text_id = 16; // no people in city
    else if (!consider_house_covering)
        text_id = 19;
    else if (b->houses_covered <= 0)
        text_id = 17; // no employees nearby
    else if (b->houses_covered < 40)
        text_id = 20; // poor access to employees
    else if (city_labor_category(b->labor_category)->workers_allocated <= 0)
        text_id = 18; // no people allocated
    else {
        text_id = 19; // too few people allocated
    }
    if (!text_id && consider_house_covering && b->houses_covered < 40)
        text_id = 20; // poor access to employees

    return text_id;
}

void draw_employment_details(object_info* c, building* b, int y_offset, int text_id) {
    painter ctx = game.painter();
    y_offset += c->offset.y;
    ImageDraw::img_generic(ctx, image_id_from_group(GROUP_CONTEXT_ICONS) + 14, vec2i{c->offset.x + 40, y_offset + 6});
    if (text_id) {
        int width = lang_text_draw_amount(8, 12, b->num_workers, c->offset.x + 60, y_offset + 10, FONT_NORMAL_BLACK_ON_DARK);
        width += text_draw_number(model_get_building(b->type)->laborers, '(', "", c->offset.x + 70 + width, y_offset + 10, FONT_NORMAL_BLACK_ON_DARK);
        lang_text_draw(69, 0, c->offset.x + 70 + width, y_offset + 10, FONT_NORMAL_BLACK_ON_DARK);
        lang_text_draw(69, text_id, c->offset.x + 70, y_offset + 26, FONT_NORMAL_BLACK_ON_DARK);
    } else {
        int width = lang_text_draw_amount(8, 12, b->num_workers, c->offset.x + 60, y_offset + 16, FONT_NORMAL_BLACK_ON_DARK);
        width += text_draw_number(model_get_building(b->type)->laborers, '(', "", c->offset.x + 70 + width, y_offset + 16, FONT_NORMAL_BLACK_ON_DARK);
        lang_text_draw(69, 0, c->offset.x + 70 + width, y_offset + 16, FONT_NORMAL_BLACK_ON_DARK);
    }
}

static void draw_employment_farm_ph_details(object_info* c, building* b, int y_offset, int text_id) {
    painter ctx = game.painter();
    y_offset += c->offset.y;
    ImageDraw::img_generic(ctx, image_id_from_group(GROUP_CONTEXT_ICONS) + 14, vec2i{c->offset.x + 40, y_offset + 6});
    int width = lang_text_draw_multiline(177, text_id, vec2i{c->offset.x + 70, y_offset + 10}, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK_ON_DARK);
}

void window_building_draw_employment(object_info* c, int y_offset) {
    building* b = building_get(c->building_id);
    int text_id = get_employment_info_text_id(c, b, y_offset, 1);
    draw_employment_details(c, b, y_offset, text_id);
}

void window_building_draw_employment_without_house_cover(object_info* c, int y_offset) {
    building* b = building_get(c->building_id);
    int text_id = get_employment_info_text_id(c, b, y_offset, 0);
    draw_employment_details(c, b, y_offset, text_id);
}

void window_building_draw_employment_flood_farm(object_info* c, int y_offset) {
    building* b = building_get(c->building_id);
    int text_id = 5;
    if (b->num_workers >= model_get_building(b->type)->laborers) {
        text_id = 6;
    }
    draw_employment_farm_ph_details(c, b, y_offset, text_id);
}
void window_building_draw_description(object_info* c, int text_group, int text_id) {
    lang_text_draw_multiline(text_group, text_id, c->offset + vec2i{32, 56}, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK_ON_LIGHT);
}
void window_building_draw_description_at(object_info* c, int y_offset, int text_group, int text_id) {
    lang_text_draw_multiline(text_group, text_id, c->offset + vec2i{32, y_offset}, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK_ON_LIGHT);
}

void window_building_play_sound(object_info* c, const char* sound_file) {
    if (c->can_play_sound) {
        sound_speech_play_file(sound_file);
        c->can_play_sound = 0;
    }
}
