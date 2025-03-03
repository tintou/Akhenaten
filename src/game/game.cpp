#include "game.h"

#include "building/construction/build_planner.h"
#include "building/model.h"
#include "core/game_environment.h"
#include "core/random.h"
#include "core/profiler.h"
#include "core/log.h"
#include "editor/editor.h"
#include "game/file_editor.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/tick.h"
#include "graphics/elements/warning.h"
#include "graphics/font.h"
#include "graphics/image.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "config/config.h"
#include "config/hotkeys.h"
#include "io/gamefiles/lang.h"
#include "content/content.h"
#include "mission.h"
#include "scenario/events.h"
#include "scenario/scenario.h"
#include "sound/city.h"
#include "sound/system.h"
#include "translation/translation.h"
#include "window/city.h"
#include "window/editor/map.h"
#include "window/logo.h"
#include "window/main_menu.h"
#include "graphics/view/view.h"
#include "platform/renderer.h"

namespace {
    static const time_millis MILLIS_PER_TICK_PER_SPEED[] = {0, 20, 35, 55, 80, 110, 160, 240, 350, 500, 700};
    static time_millis last_update;
}

game_t game;

void game_t::animation_timers_init() {
    for (auto &timer: animation_timers) {
        timer.last_update = 0;
        timer.should_update = false;
    }
}

void game_t::animation_timers_update() {
    time_millis now_millis = time_get_millis();
    for (auto &timer : animation_timers) {
        timer.should_update = false;
    }

    unsigned int delay_millis = 0;
    for (auto &timer: animation_timers) {
        if (now_millis - timer.last_update >= delay_millis) {
            timer.should_update = true;
            timer.last_update = now_millis;
        }
        delay_millis += 20;
    }
}

bool game_t::animation_should_advance(uint32_t speed) {
    if (!animation || paused) {
        return false;
    }

    if (speed >= MAX_ANIM_TIMERS) {
        return false;
    }

    return animation_timers[speed].should_update;
}

::painter game_t::painter() {
    ::painter ctx;
    ctx.view = &city_view_data_unsafe();
    ctx.renderer = graphics_renderer()->renderer();
    ctx.global_render_scale = graphics_renderer()->scale();

    return ctx;
}


static int is_unpatched(void) {
    const uint8_t* delete_game = lang_get_string(1, 6);
    const uint8_t* option_menu = lang_get_string(2, 0);
    const uint8_t* difficulty_option = lang_get_string(2, 6);
    const uint8_t* help_menu = lang_get_string(3, 0);
    // Without patch, the difficulty option string does not exist and
    // getting it "falls through" to the next text group, or, for some
    // languages (pt_BR): delete game falls through to option menu
    return difficulty_option == help_menu || delete_game == option_menu;
}

static encoding_type update_encoding(void) {
    int language = locale_determine_language();
    encoding_type encoding = encoding_determine(language);
    logs::info("Detected encoding: %i", encoding);
    font_set_encoding(encoding);
    translation_load(language);
    return encoding;
}

static bool reload_language(int is_editor, int reload_images) {
    if (!lang_load(is_editor)) {
        if (is_editor)
            logs::error("'c3_map.eng' or 'c3_map_mm.eng' files not found or too large.");
        else
            logs::error("'c3.eng' or 'c3_mm.eng' files not found or too large.");
        return false;
    }
    encoding_type encoding = update_encoding();

    if (!image_set_font_pak(encoding)) {
        logs::error("unable to load font graphics");
        return false;
    }

    return true;
}

static int get_elapsed_ticks() {
    if (game.paused || !city_has_loaded) {
        return 0;
    }

    int game_speed_index = 0;
    int ticks_per_frame = 1;
    switch (window_get_id()) {
    default:
        return 0;

    case WINDOW_CITY:
    case WINDOW_CITY_MILITARY:
    case WINDOW_SLIDING_SIDEBAR:
    case WINDOW_OVERLAY_MENU:
    case WINDOW_BUILD_MENU:
        game_speed_index = (100 - g_settings.game_speed) / 10;
        if (game_speed_index >= 10) {
            return 0;
        } else if (game_speed_index < 0) {
            ticks_per_frame = g_settings.game_speed / 100;
            game_speed_index = 0;
        }
        break;

    case WINDOW_EDITOR_MAP:
        game_speed_index = 3; // 70%, nice speed for flag animations
        break;
    }

    if (Planner.in_progress) {
        return 0;
    }

    if (scroll_in_progress() && !scroll_is_smooth())
        return 0;

    time_millis now = time_get_millis();
    time_millis diff = now - last_update;
    if (diff < MILLIS_PER_TICK_PER_SPEED[game_speed_index] + 2)
        return 0;

    last_update = now;
    return ticks_per_frame;
}

bool game_pre_init(void) {
    if (!lang_load(0)) {
        return false;
    }

    logs::switch_output(vfs::platform_file_manager_get_base_path());
    update_encoding();
    g_settings.load(); // c3.inf
    config_load();   // akhenaten.ini
    hotkey_config_load();
    scenario_settings_init();
    random_init();

    game.paused = false;
    return true;
}

bool game_init() {
    int missing_fonts = 0;
    if (!image_set_font_pak(encoding_get())) {
        logs::error("unable to load font graphics");
        if (encoding_get() == ENCODING_KOREAN)
            missing_fonts = 1;
        else
            return false;
    }

    if (!image_load_paks()) {
        logs::error("unable to load main graphics");
        return false;
    }

    if (!model_load()) {
        logs::error("unable to load model data");
        return false;
    }

    if (!eventmsg_load()) {
        logs::error("unable to load eventmsg.txt");
        return false;
    }
    if (!eventmsg_auto_phrases_load()) {
        logs::error("unable to load event auto reason phrases");
        return false;
    }
    if (!game_load_campaign_file()) {
        logs::error("unable to load campaign data");
        return false;
    }

    //    mods_init();
    sound_system_init();
    game_state_init();
    window_logo_show(missing_fonts ? MESSAGE_MISSING_FONTS : (is_unpatched() ? MESSAGE_MISSING_PATCH : MESSAGE_NONE));

    return true;
}
bool game_init_editor() {
    if (!reload_language(1, 0))
        return false;

    game_file_editor_clear_data();
    game_file_editor_create_scenario(2);

    if (city_view_is_sidebar_collapsed())
        city_view_toggle_sidebar();

    editor_set_active(1);
    window_editor_map_show();
    return true;
}
void game_exit_editor() {
    if (!reload_language(0, 0))
        return;
    editor_set_active(0);
    window_main_menu_show(1);
}
int game_reload_language() {
    return reload_language(0, 1);
}
void game_run() {
    OZZY_PROFILER_SECTION("Game/Run");
    game.animation_timers_update();

    int num_ticks = get_elapsed_ticks();
    for (int i = 0; i < num_ticks; i++) {
        game_tick_run();

        if (window_is_invalid()) {
            break;
        }
    }

    if (window_is(WINDOW_CITY)) {
        anti_scum_random_15bit();
    }
}

void game_frame_draw() {
    OZZY_PROFILER_SECTION("Render/Frame");
    window_draw(false);
}

void game_frame_end() {
    game.animation = true;
}

void game_sound_frame() {
    OZZY_PROFILER_SECTION("Sound/Frame");
    sound_city_play();
}

void game_handle_input_frame() {
    OZZY_PROFILER_SECTION("Input/Frame/Current");
    const mouse *m = mouse_get();
    const hotkeys *h = hotkey_state();

    window_type* w = window_current();
    w->handle_input(m, h);
    tooltip_handle(m, w->get_tooltip);
}

void game_draw_frame_warning() {
    OZZY_PROFILER_SECTION("Render/Frame/Warning");
    warning_draw();
}

void game_handle_input_after() {
    OZZY_PROFILER_SECTION("Input/Frame/After");
    window_update_input_after();
}

void game_exit() {
    video_shutdown();
    g_settings.save();
    config_save();
    sound_system_shutdown();
}
