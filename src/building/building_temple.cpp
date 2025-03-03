#include "building_temple.h"

#include "building/building.h"
#include "city/object_info.h"
#include "game/resource.h"
#include "graphics/elements/panel.h"
#include "graphics/elements/lang_text.h"
#include "graphics/view/view.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "io/gamefiles/lang.h"
#include "config/config.h"
#include "window/building/common.h"
#include "window/building/figures.h"
#include "sound/sound_building.h"
#include "graphics/animation.h"
#include "game/game.h"
#include "city/labor.h"
#include "widget/city/ornaments.h"

struct building_temple_osiris : public building_impl {
    building_temple_osiris(building &b) : building_impl(b) {}
    BUILDING_METAINFO(BUILDING_TEMPLE_OSIRIS, building_temple_osiris);
};
struct building_temple_ra : public building_impl {
    building_temple_ra(building &b) : building_impl(b) {}
    BUILDING_METAINFO(BUILDING_TEMPLE_RA, building_temple_ra);
};
struct building_temple_ptah : public building_impl {
    building_temple_ptah(building &b) : building_impl(b) {}
    BUILDING_METAINFO(BUILDING_TEMPLE_PTAH, building_temple_ptah);
};
struct building_temple_seth : public building_impl {
    building_temple_seth(building &b) : building_impl(b) {}
    BUILDING_METAINFO(BUILDING_TEMPLE_SETH, building_temple_seth);
};
struct building_temple_bast : public building_impl {
    building_temple_bast(building &b) : building_impl(b) {}
    BUILDING_METAINFO(BUILDING_TEMPLE_BAST, building_temple_bast);
};

buildings::model_t<building_temple_osiris> temple_osiris_m;
buildings::model_t<building_temple_ra> temple_ra_m;
buildings::model_t<building_temple_ptah> temple_ptah_m;
buildings::model_t<building_temple_seth> temple_seth_m;
buildings::model_t<building_temple_bast> temple_bast_m;

ANK_REGISTER_CONFIG_ITERATOR(config_load_building_temples);
void config_load_building_temples() {
    temple_osiris_m.load();
    temple_ra_m.load();
    temple_ptah_m.load();
    temple_seth_m.load();
    temple_bast_m.load();
}

static void building_temple_draw_temple(object_info& c, const char* type, int group_id, int image_offset) {
    c.help_id = 67;
    window_building_play_sound(&c, snd::get_building_info_sound(type));
    outer_panel_draw(c.offset, c.width_blocks, c.height_blocks);
    lang_text_draw_centered(group_id, 0, c.offset.x, c.offset.y + 12, 16 * c.width_blocks, FONT_LARGE_BLACK_ON_LIGHT);
    inner_panel_draw(c.offset.x + 16, c.offset.y + 56, c.width_blocks - 2, 4);
    window_building_draw_employment(&c, 62);
    painter ctx = game.painter();
    if (c.has_road_access) {
        ImageDraw::img_generic(ctx, image_offset + image_id_from_group(GROUP_PANEL_WINDOWS), c.offset.x + 190, c.offset.y + 16 * c.height_blocks - 118);
    } else {
        window_building_draw_description_at(c, 16 * c.height_blocks - 128, 69, 25);
    }
}

e_overlay building_temple::get_overlay() const {
    switch(type()) {
    case BUILDING_TEMPLE_OSIRIS: return OVERLAY_RELIGION_OSIRIS;
    case BUILDING_TEMPLE_RA: return OVERLAY_RELIGION_RA;
    case BUILDING_TEMPLE_PTAH: return OVERLAY_RELIGION_PTAH;
    case BUILDING_TEMPLE_SETH: return OVERLAY_RELIGION_SETH;
    case BUILDING_TEMPLE_BAST: return OVERLAY_RELIGION_BAST;
    }

    return OVERLAY_NONE;
}

e_sound_channel_city building_temple::sound_channel() const {
    switch (type()) {
    case BUILDING_TEMPLE_OSIRIS: return SOUND_CHANNEL_CITY_TEMPLE_OSIRIS;
    case BUILDING_TEMPLE_RA: return SOUND_CHANNEL_CITY_TEMPLE_RA;
    case BUILDING_TEMPLE_PTAH: return SOUND_CHANNEL_CITY_TEMPLE_PTAH;
    case BUILDING_TEMPLE_SETH: return SOUND_CHANNEL_CITY_TEMPLE_SETH;
    case BUILDING_TEMPLE_BAST: return SOUND_CHANNEL_CITY_TEMPLE_BAST;
    }

    return SOUND_CHANNEL_CITY_NONE;
}

void building_temple::window_info_background(object_info &c) {
    switch (type()) {
    case BUILDING_TEMPLE_OSIRIS:
        building_temple_draw_temple(c, "temple_osiris", 92, 21);
        break;

    case BUILDING_TEMPLE_RA:
        building_temple_draw_temple(c, "temple_ra", 93, 22);
        break;

    case BUILDING_TEMPLE_PTAH:
        building_temple_draw_temple(c, "temple_ptah", 94, 23);
        break;

    case BUILDING_TEMPLE_SETH:
        building_temple_draw_temple(c, "temple_seth", 95, 24);
        break;

    case BUILDING_TEMPLE_BAST:
        building_temple_draw_temple(c, "temple_bast", 96, 25);
        break;
    }
}

void building_temple::spawn_figure() {
    if (is_main()) {
        common_spawn_roamer(FIGURE_PRIEST, 50, FIGURE_ACTION_125_ROAMING);
    }
}

bool building_temple::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    building_draw_normal_anim(ctx, point, &base, tile, anim("work"), color_mask);

    return true;
}

const building_impl::static_params &building_temple::params() const {
    switch(type()) {
    case BUILDING_TEMPLE_OSIRIS: return temple_osiris_m;
    case BUILDING_TEMPLE_RA: return temple_ra_m;
    case BUILDING_TEMPLE_PTAH: return temple_ptah_m;
    case BUILDING_TEMPLE_SETH: return temple_seth_m;
    case BUILDING_TEMPLE_BAST: return temple_bast_m;
    }

    return building_impl::params();
}

const animation_t &building_temple::anim(pcstr key) const {
    return params().anim[key];
}
