#pragma once

#include "figuretype/figure_entertainer.h"

class figure_juggler : public figure_entertainer {
public:
    figure_juggler(figure *f) : figure_entertainer(f) {}

    virtual void on_create() override {}
    virtual void update_shows() override;
    virtual svector<e_building_type, 4> allow_venue_types() const override;
    virtual e_figure_sound phrase() const override { return {FIGURE_JUGGLER, "juggler"}; }
    virtual sound_key phrase_key() const override;
    virtual int provide_service() override;
    virtual e_overlay get_overlay() const override { return OVERLAY_BOOTH; }
    virtual figure_sound_t get_sound_reaction(pcstr key) const override;
};