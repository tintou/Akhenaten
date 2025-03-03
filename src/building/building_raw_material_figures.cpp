#include "building/building.h"

#include "core/random.h"
#include "figure/figure.h"

void building::spawn_figure_wood_cutters() {
    check_labor_problem();
    if (has_road_access) {
        common_spawn_labor_seeker(100);
        int pct_workers = worker_percentage();
        int spawn_delay = figure_spawn_timer();
        if (spawn_delay == -1) {
            return;
        }

        figure_spawn_delay++;
        if (figure_spawn_delay > spawn_delay) {
            figure_spawn_delay = 0;

            if (can_spawn_gatherer(FIGURE_LUMBERJACK, data.industry.max_gatheres, 50)) {
                auto f = create_figure_generic(FIGURE_LUMBERJACK, ACTION_8_RECALCULATE, BUILDING_SLOT_SERVICE, DIR_4_BOTTOM_LEFT);
                random_generate_next();
                f->wait_ticks = random_short() % 30; // ok
            }
        }
    }
    common_spawn_goods_output_cartpusher();
}