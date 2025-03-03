#pragma once

#include "figuretype/figure_entertainer.h"

class figure_dancer : public figure_entertainer {
public:
    figure_dancer(figure *f) : figure_entertainer(f) {}

    virtual void on_create() override {}
    virtual void update_shows() override;
    virtual figure_dancer *dcast_dancer() override { return this; }
    virtual svector<e_building_type, 4> allow_venue_types() const override { return {BUILDING_PAVILLION}; }
    virtual e_figure_sound phrase() const override { return {FIGURE_DANCER, "dancer"}; }
    virtual sound_key phrase_key() const override;
    virtual int provide_service() override;
    virtual e_overlay get_overlay() const override { return OVERLAY_PAVILION; }
    virtual figure_sound_t get_sound_reaction(pcstr key) const override;
};