#include "labor.h"

#include "city/finance.h"
#include "city/labor.h"
#include "core/calc.h"
#include "graphics/image.h"
#include "graphics/graphics.h"
#include "graphics/elements/arrow_button.h"
#include "graphics/elements/generic_button.h"
#include "graphics/view/view.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/labor_priority.h"
#include "game/game.h"

#define ADVISOR_HEIGHT 26

static void arrow_button_wages(int is_down, int param2);
static void button_priority(int category, int param2);

static generic_button category_buttons[] = {
  {40, 77, 560, 22, button_priority, button_none, 0, 0},
  {40, 102, 560, 22, button_priority, button_none, 1, 0},
  {40, 127, 560, 22, button_priority, button_none, 2, 0},
  {40, 152, 560, 22, button_priority, button_none, 3, 0},
  {40, 177, 560, 22, button_priority, button_none, 4, 0},
  {40, 202, 560, 22, button_priority, button_none, 5, 0},
  {40, 227, 560, 22, button_priority, button_none, 6, 0},
  {40, 252, 560, 22, button_priority, button_none, 7, 0},
  {40, 277, 560, 22, button_priority, button_none, 8, 0},
};

static arrow_button wage_buttons[] = {
    {158, 354, 17, 24, arrow_button_wages, 1, 0}, {182, 354, 15, 24, arrow_button_wages, 0, 0}
};

static int focus_button_id;
static int arrow_button_focus;

static int draw_background() {
    painter ctx = game.painter();
    outer_panel_draw(vec2i{0, 0}, 40, ADVISOR_HEIGHT);
    ImageDraw::img_generic(ctx, image_id_from_group(GROUP_ADVISOR_ICONS), vec2i{10, 10});

    lang_text_draw(50, 0, 60, 12, FONT_LARGE_BLACK_ON_LIGHT);

    // table headers
    lang_text_draw(50, 21, 60, 56, FONT_SMALL_PLAIN);
    lang_text_draw(50, 22, 170, 56, FONT_SMALL_PLAIN);
    lang_text_draw(50, 23, 400, 56, FONT_SMALL_PLAIN);
    lang_text_draw(50, 24, 500, 56, FONT_SMALL_PLAIN);

    // xx employed, yy unemployed
    int width = text_draw_number(city_labor_workers_employed(), '@', " ", 32, 320, FONT_NORMAL_BLACK_ON_LIGHT);
    width += lang_text_draw(50, 12, 32 + width, 320, FONT_NORMAL_BLACK_ON_LIGHT);
    width += text_draw_number(city_labor_workers_unemployed(), '@', " ", 50 + width, 320, FONT_NORMAL_BLACK_ON_LIGHT);
    width += lang_text_draw(50, 13, 50 + width, 320, FONT_NORMAL_BLACK_ON_LIGHT);
    text_draw_number(city_labor_unemployment_percentage(), '@', "%)", 50 + width, 320, FONT_NORMAL_BLACK_ON_LIGHT);

    // wages panel
    inner_panel_draw(64, 350, 32, 2);
    lang_text_draw(50, 14, 70, 359, FONT_NORMAL_WHITE_ON_DARK);
    width = text_draw_number(city_labor_wages(), '@', " ", 230, 359, FONT_NORMAL_WHITE_ON_DARK);
    width += lang_text_draw(50, 15, 230 + width, 359, FONT_NORMAL_WHITE_ON_DARK);
    width += lang_text_draw(50, 18, 230 + width, 359, FONT_NORMAL_WHITE_ON_DARK);
    text_draw_number(city_labor_wages_rome(), '@', " )", 230 + width, 359, FONT_NORMAL_WHITE_ON_DARK);

    // estimated wages
    width = lang_text_draw(50, 19, 64, 390, FONT_NORMAL_BLACK_ON_LIGHT);
    text_draw_money(city_finance_estimated_wages(), 64 + width, 390, FONT_NORMAL_BLACK_ON_LIGHT);

    return ADVISOR_HEIGHT;
}

static void draw_foreground() {
    painter ctx = game.painter();
    arrow_buttons_draw(0, 0, wage_buttons, 2);

    inner_panel_draw(32, 70, 36, 15);

    for (int i = 0; i < 9; i++) {
        int focus = i == focus_button_id - 1;
        int y_offset = 82 + 25 * i;
        button_border_draw(40, 77 + 25 * i, 560, 22, focus);
        const labor_category_data* cat = city_labor_category(i);
        if (cat->priority) {
            ImageDraw::img_generic(ctx, image_id_from_group(GROUP_LABOR_PRIORITY_LOCK), vec2i{70, y_offset - 2});
            text_draw_number(cat->priority, '@', " ", 90, y_offset, FONT_NORMAL_WHITE_ON_DARK);
        }
        lang_text_draw(50, i + 1, 170, y_offset, FONT_NORMAL_WHITE_ON_DARK);
        text_draw_number(cat->workers_needed, '@', " ", 410, y_offset, FONT_NORMAL_WHITE_ON_DARK);
        e_font font = FONT_NORMAL_WHITE_ON_DARK;
        if (cat->workers_needed != cat->workers_allocated) {
            font = FONT_NORMAL_YELLOW;
        }

        text_draw_number(cat->workers_allocated, '@', " ", 510, y_offset, font);
    }
}

static int handle_mouse(const mouse* m) {
    if (generic_buttons_handle_mouse(m, 0, 0, category_buttons, 9, &focus_button_id))
        return 1;

    return arrow_buttons_handle_mouse(m, 0, 0, wage_buttons, 2, &arrow_button_focus);
}

static void arrow_button_wages(int is_down, int param2) {
    city_labor_change_wages(is_down ? -1 : 1);
    city_finance_estimate_wages();
    city_finance_calculate_totals();
    window_invalidate();
}

static void button_priority(int category, int param2) {
    window_labor_priority_show(category);
}

static int get_tooltip_text(void) {
    if (focus_button_id)
        return 90;
    else if (arrow_button_focus)
        return 91;
    else {
        return 0;
    }
}

const advisor_window* window_advisor_labor(void) {
    static const advisor_window window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
