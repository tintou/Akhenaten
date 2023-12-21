#include "soldier_strength.h"

#include "figure/figure.h"
#include "grid/figure.h"
#include "grid/grid.h"
#include "grid/routing/routing.h"

static grid_xx strength = {0, {FS_UINT8, FS_UINT8}};

void map_soldier_strength_clear(void) {
    map_grid_clear(&strength);
}
void map_soldier_strength_add(int x, int y, int radius, int amount) {
    grid_area area = map_grid_get_area(tile2i(x, y), 1, radius);

    map_grid_area_foreach(area.tmin, area.tmax, [&] (tile2i tile) {
        int grid_offset = tile.grid_offset();
        int v = map_grid_get(&strength, grid_offset);
        map_grid_set(&strength, grid_offset, v + amount);
        if (map_has_figure_at(grid_offset)) {
            if (figure_get(map_figure_id_get(grid_offset))->is_legion())
                map_grid_set(&strength, grid_offset, v + amount + 2);
        }
    });
}

int map_soldier_strength_get(int grid_offset) {
    return map_grid_get(&strength, grid_offset);
}

int map_soldier_strength_get_max(int x, int y, int radius, tile2i &out) {
    grid_area area = map_grid_get_area(tile2i(x, y), 1, radius);

    int max_value = 0;
    tile2i max_tile(0, 0);
    map_grid_area_foreach(area.tmin, area.tmax, [&] (tile2i tile) {
        int grid_offset = tile.grid_offset();
        if (map_routing_distance(grid_offset) > 0 && map_grid_get(&strength, grid_offset) > max_value) {
            max_value = map_grid_get(&strength, grid_offset);
            max_tile = tile;
        }
    });

    if (max_value > 0) {
        out = max_tile;
        return true;
    }

    return false;
}
