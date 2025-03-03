#pragma once

#include "figure/figure.h"

class figure_carrier : public figure_impl {
public:
    figure_carrier(figure *f) : figure_impl(f) {}

    void load_resource(e_resource resource, int amount);
    int dump_resource(int amount);
};

class figure_cartpusher : public figure_carrier {
public:
    figure_cartpusher(figure *f) : figure_carrier(f) {}

    virtual void on_create() override {}
    virtual void figure_before_action() override;
    virtual void figure_action() override;
    virtual void figure_draw(painter &ctx, vec2i pixel, int highlight, vec2i* coord_out) override;
    virtual e_figure_sound phrase() const override { return {FIGURE_CART_PUSHER, "cartpusher"}; }
    virtual e_overlay get_overlay() const override { return OVERLAY_NONE; }
    virtual sound_key phrase_key() const override;

    virtual figure_cartpusher *dcast_cartpusher() override { return this; }

    void do_deliver(bool storageyard_cart, int action_done);
    void do_retrieve(int action_done);
    void calculate_destination(bool warehouseman);

    void determine_deliveryman_destination();
    void determine_granaryman_destination();
    void determine_storageyard_cart_destination();
};